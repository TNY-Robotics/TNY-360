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
            <UButton class="w-22" v-if="calibrationStateModel !== 'CALIBRATING'"
                :label="'Calibrate'" variant="soft" @click="sendCalibrate(props.index)"
            />
            <UButton class="relative w-22 overflow-hidden" v-if="calibrationStateModel === 'CALIBRATING'"
                disabled :label="'Calibrating'" variant="soft"
                color="warning"
            >
                <p class="w-full text-center">. . .</p>
                <span class="absolute top-0 left-0 h-full bg-yellow-500" :style="`width: ${calibrationProgressModel}%`" />
            </UButton>
        </div>
    </div>
</template>

<script lang="ts" setup>
const remote = useRemote();

const props = defineProps<{
    index: number;
    name: string;
    angle: number;
    range: number[];
}>();

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