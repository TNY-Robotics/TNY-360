<template>
    <div class="flex flex-col space-y-6 border-2 border-slate-200 dark:border-slate-700 p-4 rounded-lg">
        <div class="flex justify-between items-center">
            <div class="flex space-x-4">
                <p class="text-lg font-semibold whitespace-nowrap "> {{ name }} </p>
                <UButton class="w-fit h-fit" :color="enabledLoading? 'neutral' : (enabled? 'success': 'error')" variant="soft" @click="toggleEnabled" :icon="enabled? 'i-lucide-check' : 'i-lucide-x'" trailing :loading="enabledLoading">
                    {{ enabled? 'Enabled' : 'Disabled' }}
                </UButton>
            </div>
            <UButton icon="lucide:pocket-knife" color="neutral" variant="soft" @click="onAdvancedClicked" />
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
                <UButton class="w-fit h-fit" variant="soft" disabled :color="getCalibColor(calibrationState)" :label="getCalibLabel(calibrationState)"/>
            </div>
            <div class="flex justify-between items-center">
                <UButton class="w-fit h-fit" variant="soft" label="Calibrate" icon="i-lucide-ruler" @click="onCalibrateClicked" />
                <UButton class="w-fit h-fit" v-if="calibrationState === MotorCalibrationState.Calibrated" variant="soft" color="error" :loading="deleteCalibBtnLoading"
                    label="Delete Calibration" icon="i-lucide-trash" trailing @click="onDeleteCalibrationClicked" />
            </div>
        </div>
    </div>
    <JointCalibrationModal :index="driverIndex" :name="name" v-model:open="calibrationModalOpen" />
    <JointMotorAdvancedModal :driverIndex="driverIndex" :readerIndex="readerIndex" :name="name" v-model:open="advancedModalOpen" />
</template>

<script lang="ts" setup>
import { MotorCalibrationState } from '@tny-robotics/sdk';
import { RAD2DEG } from 'three/src/math/MathUtils';

const tny = useTNY360();

const props = defineProps<{
    driverIndex: number;
    readerIndex: number;
    name: string;
    range: number[];
    shouldUpdate: boolean;
}>();

const blockUpdatesModel = defineModel<boolean>('blockUpdates', { required: true });

const advancedModalOpen = ref(false);
function onAdvancedClicked() {
    advancedModalOpen.value = true;
}
watch(advancedModalOpen, (newVal) => {
    blockUpdatesModel.value = newVal;
});

const enabled = ref(false);
const enabledLoading = ref(false);
async function toggleEnabled() {
    await sendEnabled(props.driverIndex, !enabled.value);
    enabled.value = !enabled.value;
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
        await tny.value?.joint.setAngle(props.driverIndex, newAngle * Math.PI / 180, true);
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

const deleteCalibBtnLoading = ref(false);
async function onDeleteCalibrationClicked() {
    deleteCalibBtnLoading.value = true;
    try {
        await tny.value?.motor.deleteCalibrationData(props.driverIndex);
    } catch (err) {
        console.error("Error deleting calibration data :", err);
    }
    deleteCalibBtnLoading.value = false;
    calibrationState.value = await tny.value?.motor.getCalibrationState(props.driverIndex) || MotorCalibrationState.Uncalibrated;
}

watch(calibrationModalOpen, async (newVal) => {
    calibrationState.value = await tny.value?.motor.getCalibrationState(props.driverIndex) || MotorCalibrationState.Uncalibrated;
    enabled.value = await tny.value?.joint.getEnabled(props.driverIndex) || false;
});

let shouldFetchInfos = false;
async function fetchInfosContinuous() {
    if (!props.shouldUpdate) {
        // skip updates
        if (shouldFetchInfos) setTimeout(fetchInfosContinuous, 500);
        return;
    }

    try {
        enabled.value = await tny.value?.joint.getEnabled(props.driverIndex) || false;
        calibrationState.value = await tny.value?.motor.getCalibrationState(props.driverIndex) || MotorCalibrationState.Uncalibrated;
        modelAngle.value = Math.round((await tny.value?.joint.getModelAngle(props.driverIndex) ?? 0) * RAD2DEG);
        feedbackAngle.value = Math.round((await tny.value?.joint.getFeedbackAngle(props.driverIndex) ?? 0) * RAD2DEG);
        predictedAngle.value = Math.round((await tny.value?.joint.getEstimatedAngle(props.driverIndex) ?? 0) * RAD2DEG);
    } catch (err) {
        if ((err as Error).message.toLowerCase().includes('timed out')) {
            // robot is overloaded, wait a bit
            await new Promise(resolve => setTimeout(resolve, 1000));
        } else {
            console.error("Error fetching joint infos:", err);
        }
    }

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