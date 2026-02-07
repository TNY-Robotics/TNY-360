<template>
    <div class="flex space-x-4 items-center">
        <h3 class="text-lg font-medium w-16"> {{ name }} </h3>
        <div class="relative w-full">
            <USlider
                class="absolute top-0 left-0 pointer-events-none"
                :min="range[0]"
                :max="range[1]"
                :step="1"
                v-model="angleRealModel"
                color="neutral"
            />
            <USlider
                :min="range[0]"
                :max="range[1]"
                :step="1"
                v-model="angleModel"
            />
        </div>
        <p ref="angleText" class="w-12 text-right font-mono"> {{ angleModel }}° </p>
        <UButton :color="enabledModel ? 'success' : 'error'" variant="soft" @click="toggleEnabled">
            {{ enabledModel ? 'Enabled' : 'Disabled' }}
        </UButton>
        <div>
            <UDropdownMenu :items="menuItems">
                <UButton class="w-22" v-if="calibrationStateModel !== 'CALIBRATING'"
                    :label="'Calibrate'" variant="soft"
                />
            </UDropdownMenu>
            <UButton class="relative w-22 overflow-hidden" v-if="calibrationStateModel === 'CALIBRATING'"
                disabled :label="'Calibrating'" variant="soft"
                color="warning"
            >
                <p class="w-full text-center">. . .</p>
                <span class="absolute top-0 left-0 h-full bg-yellow-500" :style="`width: ${calibrationProgressModel}%`" />
            </UButton>
        </div>
    </div>

    <UModal v-model:open="manualCalibModalOpen" :title="`Manual Calibration - ${name}`">
        <template #body>
            <p class="mb-4">
                Manually move the joint to the position stops and click the minimum or maximum buttons to set the range.
            </p>
            <div class="flex flex-col w-full pb-6">
                <p class="text-center pb-2 w-full">
                    {{ pwmValue }}
                </p>
                <USlider
                    class="w-full"
                    :min="pwmRange[0]"
                    :max="pwmRange[1]"
                    :step="1"
                    v-model="pwmValue"
                />
            </div>
            <div class="flex justify-between w-full">
                <UButton @click="setMinPosition(props.index)" trailing
                    :color="minPositionSet? 'success': 'neutral'"
                    :icon="minPositionSet? 'i-lucide-check': ''"
                >
                    Set Minimum
                </UButton>
                <UButton @click="setMaxPosition(props.index)"
                    :color="maxPositionSet? 'success': 'neutral'"
                    :icon="maxPositionSet? 'i-lucide-check': ''"
                >
                    Set Maximum
                </UButton>
            </div>
        </template>
    </UModal>
</template>

<script lang="ts" setup>
import type { DropdownMenuItem } from '@nuxt/ui';

const remote = useRemote();

const props = defineProps<{
    index: number;
    name: string;
    angle: number;
    range: number[];
}>();

const pwmRange: number[] = [4096*(0.4/20), 4096*(2.6/20)]; // convert from microseconds to 0-4096 range (50hz - 20ms period)
const pwmValue = ref(((pwmRange?.[0] ?? 0) + (pwmRange?.[1] ?? 0)) / 2); // default to middle position

watch(pwmValue, (newPwm) => {
    remote.setJointPWM(props.index, newPwm).then(() => {
        console.log('Set PWM for joint', props.index, 'to', newPwm);
    }).catch((err) => {
        console.error('Failed to set PWM for joint', props.index, err);
    });
});

const manualCalibModalOpen = ref(false);
const menuItems = ref<DropdownMenuItem[][]>([
    [
        {
            label: 'Automatic',
            onClick: () => sendCalibrate(props.index),
        },
        {
            label: 'Manual',
            onClick: () => { manualCalibModalOpen.value = true; }
        },
    ],
]);

watch(manualCalibModalOpen, (isOpen) => {
    if (isOpen) {
        remote.setJointCalibrationState(props.index, 'CALIBRATING').then(() => {
            console.log('Set joint', props.index, 'to calibrating state');
        }).catch((err) => {
            console.error('Failed to set joint', props.index, 'to calibrating state:', err);
            alert('Failed to start manual calibration: ' + err.message);
            manualCalibModalOpen.value = false;
        });
    } else {
        remote.setJointCalibrationState(props.index, 'CALIBRATED').then(() => {
            console.log('Set joint', props.index, 'to calibrated state');
        }).catch((err) => {
            console.error('Failed to set joint', props.index, 'to calibrated state:', err);
            alert('Failed to end manual calibration: ' + err.message);
        });
    }
});

const maxPositionSet = ref(false);
function setMaxPosition(jointIndex: number) {
    remote.declareJointMaximum(jointIndex).then(() => {
        maxPositionSet.value = true;
        setTimeout(() => {
            maxPositionSet.value = false;
        }, 2000);
    }).catch((err) => {
        console.error('Failed to set maximum position for joint', jointIndex, err);
        alert('Failed to set maximum position: ' + err.message);
    });
}

const minPositionSet = ref(false);
function setMinPosition(jointIndex: number) {
    remote.declareJointMinimum(jointIndex).then(() => {
        minPositionSet.value = true;
        setTimeout(() => {
            minPositionSet.value = false;
        }, 2000);
    }).catch((err) => {
        console.error('Failed to set minimum position for joint', jointIndex, err);
        alert('Failed to set minimum position: ' + err.message);
    });
}

const angleRealModel = ref(props.angle);
let angleRealModelTarget = props.angle;
watch(() => props.angle, (newAngle) => {
    angleRealModelTarget = newAngle;
});
setInterval(() => {
    // smooth transition
    angleRealModel.value += (angleRealModelTarget - angleRealModel.value) * 0.2;
}, 100);

const enabledModel = ref(false);
const calibrationStateModel = ref<CalibrationState>('UNCALIBRATED');
const calibrationProgressModel = ref(0);

onMounted(async () => {
    try {
        const enabled = await remote.getJointState(props.index);
        enabledModel.value = enabled;
    } catch (err) {
        console.error(`Failed to get joint state for joint ${props.index}:`, err);
    }
    try {
        const calibState = await remote.getCalibrationState(props.index);
        calibrationStateModel.value = calibState;
    } catch (err) {
        console.error(`Failed to get calibration state for joint ${props.index}:`, err);
    }
});

const angleModel = ref(props.angle);
const updateAngleTimeout = ref<number | null>(null);
watch(angleModel, (newAngle) => {
    if (updateAngleTimeout.value) {
        clearTimeout(updateAngleTimeout.value);
    }
    updateAngleTimeout.value = setTimeout(() => {
        sendAngle(props.index, newAngle);
    }, 200);
});

function toggleEnabled() {
    enabledModel.value = !enabledModel.value;
    sendEnabled(props.index, enabledModel.value);
}

async function sendAngle(jointIndex: number, angle: number) {
    await remote.setJointTarget(jointIndex, angle);
}

async function sendEnabled(jointIndex: number, enabled: boolean) {
    await remote.setJointState(jointIndex, enabled);
}

async function sendCalibrate(jointIndex: number) {
    calibrationStateModel.value = 'CALIBRATING';
    calibrationProgressModel.value = 0;
    await remote.calibrateJoint(jointIndex);

    // wait for calibration to finish and update progress
    while (calibrationStateModel.value === 'CALIBRATING') {
        // wait a bit, to avoid spamming the robot
        await new Promise((resolve) => setTimeout(resolve, 1000));

        const calibrationState = await remote.getCalibrationState(jointIndex);
        calibrationStateModel.value = calibrationState;

        if (calibrationState === 'CALIBRATING') {
            const progress = await remote.getCalibrationProgress(jointIndex);
            calibrationProgressModel.value = progress * 100; // convert to percentage
        }
    }
}

</script>

<style></style>