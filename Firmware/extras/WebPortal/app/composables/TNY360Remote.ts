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

    public calibrateBody(): Promise<void> {
        return this.remote.send(0x01, [], [], []).then(() => {});
    }

    public calibrateJoint(jointIndex: number): Promise<void> {
        return this.remote.send(0x02, [Type.BYTE], [], [jointIndex]).then(() => {});
    }

    public declareJointMinimum(jointIndex: number): Promise<void> {
        return this.remote.send(0x03, [Type.BYTE], [], [jointIndex]).then(() => {});
    }

    public declareJointMaximum(jointIndex: number): Promise<void> {
        return this.remote.send(0x04, [Type.BYTE], [], [jointIndex]).then(() => {});
    }
    public setJointCalibrationState(jointIndex: number, state: CalibrationState): Promise<void> {
        let stateByte: number;
        switch (state) {
            case 'UNCALIBRATED': stateByte = 0; break;
            case 'CALIBRATING': stateByte = 1; break;
            case 'CALIBRATED': stateByte = 2; break;
            default: throw new RobotError(`Unknown calibration state: ${state}`);
        }
        return this.remote.send(0x05, [Type.BYTE, Type.BYTE], [], [jointIndex, stateByte]).then(() => {});
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

    public getBodyOrientation(): Promise<{ x: number; y: number; z: number; w: number }> {
        return this.remote.send(0x28, [], Array.from({ length: 4 }, () => Type.FLOAT), []).then((args) => {
            return {
                x: RAD_TO_DEG(args[0] as number),
                y: RAD_TO_DEG(args[1] as number),
                z: RAD_TO_DEG(args[2] as number),
                w: RAD_TO_DEG(args[3] as number),
            };
        });
    }

    public getJointPWM(jointIndex: number): Promise<number> {
        return this.remote.send(0x29, [Type.BYTE], [Type.INT], [jointIndex]).then((args) => {
            return args[0] as number;
        });
    }

    public getJointVoltage(jointIndex: number): Promise<number> {
        return this.remote.send(0x2A, [Type.BYTE], [Type.INT], [jointIndex]).then((args) => {
            return args[0] as number;
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

    public setJointPWM(index: number, pwm: number): Promise<void> {
        return this.remote.send(0x67, [Type.BYTE, Type.INT], [], [index, pwm]).then(() => {});
    }
}

export function useRemote() {
    return TNY360Remote.GetInstance();
}