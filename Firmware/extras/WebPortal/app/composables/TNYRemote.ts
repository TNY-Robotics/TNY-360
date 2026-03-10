import { sizeof, TimeoutError, Type, type CommandID } from "./utils"
const toast = useToast();

export type TNYRemoteEvents = {
    connected: []
    disconnected: []
    error: []
}

export class RobotError extends Error {
    constructor(message: string) {
        super(message);
        this.name = "RobotError";
    }
}

type HandlerFunction = (...args: any[]) => void;
interface RequestHandler {
    outputTypes: Type[];
    handler: HandlerFunction;
}

const lastResponseTimes: number[] = [];
function onResponseTime(time: number) {
    lastResponseTimes.push(time);
    if (lastResponseTimes.length > 10) {
        lastResponseTimes.shift();
    }
    const avg = lastResponseTimes.reduce((a, b) => a + b, 0) / lastResponseTimes.length;
}

export class TNYRemote extends EventEmitter<TNYRemoteEvents> {
    private static Instance : TNYRemote | null = null;
    private static Ref : Ref<TNYRemote | null> = ref(null);

    public static getInstance() {
        return this.Instance;
    }

    public static getRef() {
        return this.Ref;
    }
    
    private socket!: WebSocket
    private connected: boolean = false;

    private pendingRequests: Map<number, RequestHandler> = new Map();

    private init(host: string) {
        if (TNYRemote.Instance) {
            TNYRemote.Instance.close();
        }
        TNYRemote.Instance = this;
        TNYRemote.Ref.value = this;

        this.socket = new WebSocket(`ws://${host}:5621`);

        const connectionToast = toast.add({
            title: 'Connecting to TNY-360',
            description: `Connecting to the robot with WebSockets, please wait ...`,
            color: 'warning',
            duration: 0 // Reste affiché en attendant la connexion
        });

        this.socket.addEventListener('open', () => {
            this.emit('connected');
            this.connected = true;

            toast.update(connectionToast.id, {
                title: 'Connected to TNY-360!',
                description: 'Connected through WebSockets!',
                color: 'success',
                duration: 2000
            });

            // DEV mode : store ip in localstorage for next session
            if (import.meta.dev) {
                localStorage.setItem('dev-ip', host);
            }
        });
        this.socket.addEventListener('message', async (event) => {
            const msg = event.data as Blob;
            if (msg.size < 1) {
                console.warn('Received empty message');
                return;
            }

            const view = new DataView(await msg.arrayBuffer());

            const randomId = view.getUint16(0, true) as CommandID;
            const status = view.getUint8(2) as number;
            const length = view.getUint8(3) as number;
            const requestEntry = this.pendingRequests.get(randomId);
            if (!requestEntry) {
                console.warn(`No pending request for random ID: ${randomId}`);
                return;
            }
            const { outputTypes, handler } = requestEntry;
            const expectedOutputSize = sizeof(outputTypes);
            if (msg.size - 4 < expectedOutputSize) {
                console.warn(`Insufficient data for command [random] ID ${randomId}: expected ${expectedOutputSize}, got ${msg.size - 1}`);
                return;
            }
            if (length !== expectedOutputSize) {
                console.warn(`Mismatched indicated length for command [random] ID ${randomId}: expected ${expectedOutputSize}, got ${length}`);
                return;
            }

            const outputs: any[] = [];
            let offset = 4;
            for (let type of outputTypes) {
                switch (type) {
                    case Type.BYTE:  outputs.push(view.getUint8(offset)); break;
                    case Type.INT:   outputs.push(view.getInt32(offset, true)); break;
                    case Type.INT16:   outputs.push(view.getInt16(offset, true)); break;
                    case Type.UINT16:   outputs.push(view.getUint16(offset, true)); break;
                    case Type.INT32:   outputs.push(view.getInt32(offset, true)); break;
                    case Type.UINT32:   outputs.push(view.getUint32(offset, true)); break;
                    case Type.FLOAT: outputs.push(view.getFloat32(offset, true)); break;
                    case Type.DOUBLE: outputs.push(view.getFloat64(offset, true)); break;
                    case Type.BOOL:  outputs.push(!!view.getUint8(offset)); break;
                    default: throw new Error(`Unknown type: ${type}`);
                }
                offset += sizeof(type);
            }

            handler(...outputs);
            this.pendingRequests.delete(randomId);
        });
        this.socket.addEventListener('error', (event) => {
            // DEV mode ? Then search IP in localstorage, if none, ask user for one.
            if (import.meta.dev) {
                toast.remove(connectionToast.id);
                // If there's an ip stored in localstorage, try connecting to it.
                // if there's none, ask the user for one.
                const devIp = localStorage.getItem('dev-ip');
                if (devIp) {
                    this.init(devIp);
                } else {
                    console.log("DEV mode detected. Asking for robot IP address");
                    const ip = prompt("DEV mode detected. Please enter robot IP.")
                    if (ip) this.init(ip);
                }

                // remove ip from localstorage
                // (will be added back if connection is successfull, this allows prompting only when stored ip doesn't work)
                if (localStorage.getItem('dev-ip')) {
                    localStorage.removeItem('dev-ip');
                }
            } else {
                toast.update(connectionToast.id, {
                    title: 'Failed to connnect!',
                    description: 'Sorry, something went wrong ...',
                    color: 'error',
                    duration: 4000
                });
            }

            this.emit('error');
        });
        this.socket.addEventListener('close', () => {
            if (this.connected) {
                this.emit('disconnected');
            }
            
            if (TNYRemote.Instance === this) {
                TNYRemote.Instance = null;
                TNYRemote.Ref.value = null;
            }
        });
    }

    constructor(host: string) {
        super();
        this.init(host);
    }

    public close() {
        this.socket.close();
    }

    public send(commandId: CommandID, inputTypes: Type[] = [], outputTypes: Type[] = [], args: any[] = []): Promise<any[]> {
        return new Promise((resolve, reject) => {
            if (!this.connected) {
                return reject(new Error('Not connected to robot'));
            }

            const randomId = Math.floor(Math.random() * 65536); // unique call id
            
            // Serialize command
            let bufferSize = 2 + 1 + inputTypes.reduce((sum, t) => sum + sizeof(t), 0);
            const buffer = new ArrayBuffer(bufferSize);
            const view = new DataView(buffer);
            view.setUint16(0, randomId, true);
            view.setUint8(2, commandId);
            let offset = 3;
            for (let i = 0; i < inputTypes.length; i++) {
                const type = inputTypes[i];
                const value = args[i];
                switch (type) {
                    case Type.BYTE:  view.setUint8(offset, value); break;
                    case Type.INT:   view.setInt32(offset, value, true); break;
                    case Type.INT16:   view.setInt16(offset, value, true); break;
                    case Type.UINT16:   view.setUint16(offset, value, true); break;
                    case Type.INT32:   view.setInt32(offset, value, true); break;
                    case Type.UINT32:   view.setUint32(offset, value, true); break;
                    case Type.FLOAT: view.setFloat32(offset, value, true); break;
                    case Type.DOUBLE: view.setFloat64(offset, value, true); break;
                    case Type.BOOL:  view.setUint8(offset, value ? 1 : 0); break;
                    default: return reject(new Error(`Unknown type: ${type}`));
                }
                offset += sizeof(type);
            }

            const startTime = Date.now();

            this.socket.send(buffer);

            const timeoutId = setTimeout(() => {
                this.pendingRequests.delete(randomId);
                reject(new TimeoutError());
            }, 2000);
            this.pendingRequests.set(randomId, { outputTypes, handler: (...outputs: any[]) => {
                clearTimeout(timeoutId);
                resolve(outputs);
                const endTime = Date.now();
                onResponseTime(endTime - startTime);
            }});
        });
    }
}