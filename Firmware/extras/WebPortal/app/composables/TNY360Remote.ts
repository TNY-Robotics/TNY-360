const DEG_TO_RAD = (x: number) => x * Math.PI / 180;
const RAD_TO_DEG = (x: number) => x * 180 / Math.PI;

export type CalibrationState = 'UNCALIBRATED' | 'CALIBRATING' | 'CALIBRATED';

export class TNY360Remote {
    private static Instance: TNY360Remote | null = null;
    
    public static GetInstance(): TNY360Remote {
        if (!this.Instance) {
            this.Instance = new TNY360Remote(new TNYRemote(`ws://${window.location.hostname}:5621`));
        }
        return this.Instance;
    }

    private constructor(private remote: TNYRemote) {
        this.remote = remote;
    }

    public ping(): Promise<boolean> {
        return this.remote.send(0x00, [], [Type.BOOL], []).then((args) => {
            return args[0] as boolean;
        });
    }

    public calibrateJoint(jointIndex: number): Promise<void> {
        return this.remote.send(0x02, [Type.BYTE], [], [jointIndex]).then(() => {});
    }

    public getJointState(jointIndex: number): Promise<boolean> {
        return this.remote.send(0x20, [Type.BYTE], [Type.BOOL], [jointIndex]).then((args) => {
            return args[0] as boolean;
        });
    }

    public getJointTarget(jointIndex: number): Promise<number> {
        return this.remote.send(0x21, [Type.BYTE], [Type.FLOAT], [jointIndex]).then((args) => {
            return RAD_TO_DEG(args[0] as number);
        });
    }

    public getJointPosition(jointIndex: number): Promise<number> {
        return this.remote.send(0x22, [Type.BYTE], [Type.FLOAT], [jointIndex]).then((args) => {
            return RAD_TO_DEG(args[0] as number);
        });
    }

    // feedback

    // predicted

    public getCalibrationState(jointIndex: number): Promise<CalibrationState> {
        return this.remote.send(0x25, [Type.BYTE], [Type.BYTE], [jointIndex]).then((args) => {
            const state = args[0] as number;
            switch (state) {
                case 0: return 'UNCALIBRATED';
                case 1: return 'CALIBRATING';
                case 2: return 'CALIBRATED';
                default: throw new RobotError(`Unknown calibration state: ${state}`);
            }
        });
    }

    public getCalibrationProgress(jointIndex: number): Promise<number> {
        return this.remote.send(0x26, [Type.BYTE], [Type.FLOAT], [jointIndex]).then((args) => {
            return args[0] as number;
        });
    }

    public getAllJointAngles(): Promise<number[]> {
        return this.remote.send(0x27, [], Array.from({ length: 12 }, () => Type.FLOAT), []).then((args) => {
            return args.map((angle) => RAD_TO_DEG(angle as number));
        });
    }

    public setJointState(jointIndex: number, enabled: boolean): Promise<void> {
        return this.remote.send(0x60, [Type.BYTE, Type.BOOL], [], [jointIndex, enabled]).then(() => {});
    }

    public setJointTarget(jointIndex: number, angle: number): Promise<void> {
        return this.remote.send(0x61, [Type.BYTE, Type.FLOAT], [], [jointIndex, DEG_TO_RAD(angle)]).then(() => {});
    }

    public setBodyPosture(rotX: number, rotY: number, rotZ: number, posX: number, posY: number, posZ: number): Promise<void> {
        return this.remote.send(0x65, [Type.FLOAT, Type.FLOAT, Type.FLOAT, Type.FLOAT, Type.FLOAT, Type.FLOAT], [], [
            posX*10, // robot works in mm not cm
            posY*10, // robot works in mm not cm
            posZ*10, // robot works in mm not cm
            DEG_TO_RAD(rotX),
            DEG_TO_RAD(rotY),
            DEG_TO_RAD(rotZ),
        ]).then(() => {});
    }

    public setFeetPosition(index: number, posX: number, posY: number, posZ: number): Promise<void> {
        return this.remote.send(0x66, [Type.BYTE, Type.FLOAT, Type.FLOAT, Type.FLOAT], [], [
            index,
            posX*10, // robot works in mm not cm
            posY*10, // robot works in mm not cm
            posZ*10, // robot works in mm not cm
        ]).then(() => {});
    }
}

export function useRemote() {
    return TNY360Remote.GetInstance();
}