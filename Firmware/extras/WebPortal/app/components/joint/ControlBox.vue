<template>
    <div class="flex flex-col space-y-6 border-2 border-slate-200 dark:border-slate-700 p-4 rounded-lg">
        <div class="flex justify-between items-center space-x-4">
            <p class="text-lg font-semibold"> {{ name }} </p>
            <UButton :color="enabledLoading? 'neutral' : (enabled? 'success': 'error')" variant="soft" @click="toggleEnabled" :icon="enabled? 'i-lucide-check' : 'i-lucide-x'" trailing :loading="enabledLoading">
                {{ enabled? 'Enabled' : 'Disabled' }}
            </UButton>
        </div>
        <div class="flex flex-col space-y-3">
            <div class="flex space-x-8 justify-between items-center">
                <p> Position </p>
                <p> {{ targetAngle }}° </p>
            </div>
            <USlider class="w-full" :min="range[0]" :max="range[1]" :step="1" v-model="targetAngle" />
            <div class="flex justify-between space-x-4 items-center">
                <div class="flex flex-col space-y-1">
                    <p> Model Angle </p>
                    <span class="bg-slate-200 dark:bg-slate-700 px-1 py-0.5 rounded text-center"> {{ modelAngle }}° </span>
                </div>
                <div class="flex flex-col space-y-1">
                    <p> Feedback Angle </p>
                    <span class="bg-slate-200 dark:bg-slate-700 px-1 py-0.5 rounded text-center"> {{ feedbackAngle }}° </span>
                </div>
                <div class="flex flex-col space-y-1">
                    <p> Predicted Angle </p>
                    <span class="bg-slate-200 dark:bg-slate-700 px-1 py-0.5 rounded text-center"> {{ predictedAngle }}° </span>
                </div>
            </div>
        </div>
        <div class="flex flex-col space-y-3">
            <div class="flex space-x-8 justify-between items-center">
                <p> Calibration </p>
                <UButton variant="soft" disabled :color="getCalibColor(calibrationState)" :label="getCalibLabel(calibrationState)"/>
            </div>
            <div class="flex justify-between items-center">
                <UButton variant="soft" label="Calibrate" icon="i-lucide-ruler" @click="onCalibrateClicked" />
                <!-- <UButton v-if="calibrationState === 'CALIBRATED'" variant="soft" color="error" :loading="deleteCalibBtnLoading"
                    label="Delete Calibration" icon="i-lucide-trash" trailing @click="onDeleteCalibrationClicked" /> -->
            </div>
        </div>
    </div>
    <JointCalibrationModal :index="index" :name="name" v-model:open="calibrationModalOpen" />
</template>

<script lang="ts" setup>
import { MotorCalibrationState } from '@tny-robotics/sdk';
import { RAD2DEG } from 'three/src/math/MathUtils';

const tny = useTNY360();

const props = defineProps<{
    index: number;
    name: string;
    range: number[];
}>();

const enabled = ref(false);
const enabledLoading = ref(false);
async function toggleEnabled() {
    enabled.value = !enabled.value;
    await sendEnabled(props.index, enabled.value);
}
async function sendEnabled(jointIndex: number, enabled: boolean) {
    enabledLoading.value = true;
    await tny.value?.joint.setEnabled(jointIndex, enabled, true);
    enabledLoading.value = false;
}

const targetAngle = ref(Math.round(((props.range[0]||0) + (props.range[1]||0)) / 2));
const modelAngle = ref(0);
const feedbackAngle = ref(0);
const predictedAngle = ref(0);

const onTargetAngleChangedTimeout = ref<number | null>(null);
watch(targetAngle, (newAngle) => {
    if (onTargetAngleChangedTimeout.value !== null) {
        clearTimeout(onTargetAngleChangedTimeout.value);
    }
    onTargetAngleChangedTimeout.value = setTimeout(async () => {
        await tny.value?.joint.setAngle(props.index, newAngle * Math.PI / 180, true);
    }, 100);
});

const calibrationState = ref<MotorCalibrationState>(MotorCalibrationState.Uncalibrated);
function getCalibColor(state: MotorCalibrationState) {
    return ({
        [MotorCalibrationState.Uncalibrated]: 'error',
        [MotorCalibrationState.Calibrating]: 'warning',
        [MotorCalibrationState.Calibrated]: 'success',
        [MotorCalibrationState.Error]: 'error',
    }[state]) as 'error'|'warning'|'success';
}
function getCalibLabel(state: MotorCalibrationState) {
    return ({
        [MotorCalibrationState.Uncalibrated]: 'Uncalibrated',
        [MotorCalibrationState.Calibrating]: 'Calibrating',
        [MotorCalibrationState.Calibrated]: 'Calibrated',
        [MotorCalibrationState.Error]: 'Error',
    }[state]) as string;
}

const calibrationModalOpen = ref(false);
function onCalibrateClicked() {
    calibrationModalOpen.value = true;
}

// const deleteCalibBtnLoading = ref(false);
// async function onDeleteCalibrationClicked() {
//     deleteCalibBtnLoading.value = true;
//     try {
//         // await tny.value?.motor.setCalibrationData(props.index);
//     } catch (err) {
//         console.error("Error deleting calibration data :", err);
//     }
//     deleteCalibBtnLoading.value = false;
//     calibrationState.value = await remote.getJointCalibrationState(props.index);
// }

watch(calibrationModalOpen, async (newVal) => {
    calibrationState.value = await tny.value?.motor.getCalibrationState(props.index) || MotorCalibrationState.Uncalibrated;
    enabled.value = await tny.value?.joint.getEnabled(props.index) || false;
});

let shouldFetchInfos = false;
async function fetchInfosContinuous() {
    try {
        enabled.value = await tny.value?.joint.getEnabled(props.index) || false;
        calibrationState.value = await tny.value?.motor.getCalibrationState(props.index) || MotorCalibrationState.Uncalibrated;
        modelAngle.value = Math.round((await tny.value?.joint.getModelAngle(props.index) ?? 0) * RAD2DEG);
        feedbackAngle.value = Math.round((await tny.value?.joint.getFeedbackAngle(props.index) ?? 0) * RAD2DEG);
        predictedAngle.value = Math.round((await tny.value?.joint.getEstimatedAngle(props.index) ?? 0) * RAD2DEG);
    } catch (err) { console.warn('Failed to get joint infos', err); }

    if (shouldFetchInfos) setTimeout(fetchInfosContinuous, 500);
}

onMounted(async () => {
    shouldFetchInfos = true;
    fetchInfosContinuous();
});

onUnmounted(() => {
    shouldFetchInfos = false;
})
</script>

<style></style>