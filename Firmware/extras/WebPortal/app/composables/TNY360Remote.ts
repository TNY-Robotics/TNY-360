const DEG_TO_RAD = (x: number) => x * Math.PI / 180;
const RAD_TO_DEG = (x: number) => x * 180 / Math.PI;

export type CalibrationState = 'UNCALIBRATED' | 'CALIBRATING' | 'CALIBRATED';

export interface CalibrationData {
    min_pwm: number;
    max_pwm: number;
    min_voltage: number;
    max_voltage: number;
    feedback_noise: number;
    pwm_deadband: number;
    feedback_latency_ms: number;
}
const CalibrationDataTypes = [Type.UINT16, Type.UINT16, Type.INT, Type.INT, Type.INT, Type.UINT16, Type.UINT16]

export class TNY360Remote {
    private static Instance: TNY360Remote | null = null;
    
    public static GetInstance(): TNY360Remote {
        if (!this.Instance) {
            this.Instance = new TNY360Remote(new TNYRemote(window.location.hostname));
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

    public startBodyCalibration(): Promise<void> {
        return this.remote.send(0x01, [], [], []).then(() => {});
    }
    public startJointCalibration(jointIndex: number): Promise<void> {
        return this.remote.send(0x02, [Type.BYTE], [], [jointIndex]).then(() => {});
    }
    public stopJointCalibration(jointIndex: number): Promise<void> {
        return this.remote.send(0x03, [Type.BYTE], [], [jointIndex]).then(() => {});
    }
    public setJointCalibrationData(jointIndex: number, calib_data: CalibrationData): Promise<void> {
        return this.remote.send(0x04, [Type.BYTE].concat(CalibrationDataTypes), [], [
            jointIndex,
            calib_data.min_pwm, calib_data.max_pwm,
            calib_data.min_voltage, calib_data.max_voltage, calib_data.feedback_noise,
            calib_data.pwm_deadband, calib_data.feedback_latency_ms
        ]).then(() => {});
    }
    public getJointCalibrationData(jointIndex: number): Promise<CalibrationData> {
        return this.remote.send(0x05, [Type.BYTE], CalibrationDataTypes, [jointIndex]).then((args) => {
            return {
                min_pwm: args[0], max_pwm: args[1],
                min_voltage: args[2], max_voltage: args[3], feedback_noise: args[4],
                pwm_deadband: args[5], feedback_latency_ms: args[6],
            }
        });
    }
    public deleteJointCalibration(jointIndex: number): Promise<void> {
        return this.remote.send(0x06, [Type.BYTE], [], [jointIndex]).then(() => {});
    }
    public getJointCalibrationState(jointIndex: number): Promise<CalibrationState> {
        return this.remote.send(0x07, [Type.BYTE], [Type.BYTE], [jointIndex]).then((args) => {
            const state = args[0] as number;
            switch (state) {
                case 0: return 'UNCALIBRATED';
                case 1: return 'CALIBRATING';
                case 2: return 'CALIBRATED';
                default: throw new RobotError(`Unknown calibration state: ${state}`);
            }
        });
    }
    public getJointCalibrationProgress(jointIndex: number): Promise<number> {
        return this.remote.send(0x08, [Type.BYTE], [Type.FLOAT], [jointIndex]).then((args) => {
            return args[0] as number;
        });
    }
    public getCPUUsage(): Promise<{core0: number, core1: number}> {
        return this.remote.send(0x09, [], [Type.FLOAT, Type.FLOAT], []).then((args) => {
            return {
                core0: args[0] as number,
                core1: args[1] as number
            };
        });
    }
    public getRAMUsage(): Promise<{internal_total: number, internal_used: number, psram_total: number, psram_used: number}> {
        return this.remote.send(0x0A, [], [Type.UINT32, Type.UINT32, Type.UINT32, Type.UINT32], []).then((args) => {
            return {
                internal_total: args[0] as number,
                internal_used: args[1] as number,
                psram_total: args[2] as number,
                psram_used: args[3] as number
            };
        });
    }
    public getCPUTemp(): Promise<number> {
        return this.remote.send(0x0B, [], [Type.FLOAT], []).then((args) => {
            return args[0] as number;
        });
    }


    public getJointState(jointIndex: number): Promise<boolean> {
        return this.remote.send(0x20, [Type.BYTE], [Type.BOOL], [jointIndex]).then((args) => {
            return args[0] as boolean;
        });
    }

    public getJointTargetAngle(jointIndex: number): Promise<number> {
        return this.remote.send(0x21, [Type.BYTE], [Type.FLOAT], [jointIndex]).then((args) => {
            return RAD_TO_DEG(args[0] as number);
        });
    }

    public getJointPredictedAngle(jointIndex: number): Promise<number> {
        return this.remote.send(0x22, [Type.BYTE], [Type.FLOAT], [jointIndex]).then((args) => {
            return RAD_TO_DEG(args[0] as number);
        });
    }

    public getJointFeedbackAngle(jointIndex: number): Promise<number> {
        return this.remote.send(0x23, [Type.BYTE], [Type.FLOAT], [jointIndex]).then((args) => {
            return RAD_TO_DEG(args[0] as number);
        });
    }

    public getJointModelAngle(jointIndex: number): Promise<number> {
        return this.remote.send(0x24, [Type.BYTE], [Type.FLOAT], [jointIndex]).then((args) => {
            return RAD_TO_DEG(args[0] as number);
        });
    }

    public getAllJointAngles(): Promise<number[]> {
        return this.remote.send(0x25, [], Array.from({ length: 12 }, () => Type.FLOAT), []).then((args) => {
            return args.map((angle) => RAD_TO_DEG(angle as number));
        });
    }

    public getBodyOrientation(): Promise<{ x: number; y: number; z: number; w: number }> {
        return this.remote.send(0x26, [], Array.from({ length: 4 }, () => Type.FLOAT), []).then((args) => {
            return {
                x: RAD_TO_DEG(args[0] as number),
                y: RAD_TO_DEG(args[1] as number),
                z: RAD_TO_DEG(args[2] as number),
                w: RAD_TO_DEG(args[3] as number),
            };
        });
    }

    public getJointPWM(jointIndex: number): Promise<number> {
        return this.remote.send(0x27, [Type.BYTE], [Type.INT32], [jointIndex]).then((args) => {
            return args[0] as number;
        });
    }

    public getJointVoltage(jointIndex: number): Promise<number> {
        return this.remote.send(0x28, [Type.BYTE], [Type.INT32], [jointIndex]).then((args) => {
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
        return this.remote.send(0x67, [Type.BYTE, Type.INT32], [], [index, pwm]).then(() => {});
    }

    public setMovementVelocity(tx: number, ty: number, rz: number): Promise<void> {
        return this.remote.send(0x68, [Type.FLOAT, Type.FLOAT, Type.FLOAT], [], [
            tx,
            ty,
            DEG_TO_RAD(rz),
        ]).then(() => {});
    }
}

export function useRemote() {
    return TNY360Remote.GetInstance();
}