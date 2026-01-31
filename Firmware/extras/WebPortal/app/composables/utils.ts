export type CommandID = number;
export enum Type {
    BYTE = 'byte',
    INT = 'int',
    FLOAT = 'float',
    BOOL = 'bool',
};
export function sizeof(type: Type|Type[]): number {
    if (Array.isArray(type)) {
        return type.reduce((sum, t) => sum + sizeof(t), 0);
    }
    switch (type) {
        case Type.BYTE:  return 1;
        case Type.INT:   return 4;
        case Type.FLOAT: return 4;
        case Type.BOOL:  return 1;
        default: throw new Error(`Unknown type: ${type}`);
    }
}

export class EventEmitter<Events extends Record<string, any[]>> {
    private listeners: {
        [K in keyof Events]?: ((...args: Events[K]) => void)[]
    } = {};

    on<K extends keyof Events>(event: K, callback: (...args: Events[K]) => void) {
        (this.listeners[event] ??= []).push(callback);
    }

    off<K extends keyof Events>(event: K, callback: (...args: Events[K]) => void) {
        this.listeners[event] = (this.listeners[event] ?? []).filter(cb => cb !== callback);
    }

    emit<K extends keyof Events>(event: K, ...args: Events[K]) {
        for (const cb of this.listeners[event] ?? []) {
            cb(...args);
        }
    }
}

export class TimeoutError extends Error {
    constructor(message: string = 'Request timed out') {
        super(message);
        this.name = 'TimeoutError';
    }
}
