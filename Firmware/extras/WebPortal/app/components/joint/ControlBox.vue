<template>
    <div class="flex flex-col space-y-6 border-2 border-slate-200 dark:border-slate-700 p-4 rounded-lg">
        <div class="flex justify-between items-center space-x-4">
            <p class="text-lg font-semibold"> {{ name }} </p>
            <UButton v-if="enabled" color="success" variant="soft" @click="toggleEnabled" icon="i-lucide-check" trailing>
                Enabled
            </UButton>
            <UButton v-if="!enabled" color="error" variant="soft" @click="toggleEnabled" icon="i-lucide-x" trailing>
                Disabled
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
                <UButton variant="soft" disabled :color="getCalibColor(calibrationState)" :label="calibrationState"/>
            </div>
            <div class="flex justify-between items-center">
                <UButton variant="soft" label="Calibrate" icon="i-lucide-ruler" @click="onCalibrateClicked" />
                <UButton v-if="calibrationState === 'CALIBRATED'" variant="soft" color="error" :loading="deleteCalibBtnLoading"
                    label="Delete Calibration" icon="i-lucide-trash" trailing @click="onDeleteCalibrationClicked" />
            </div>
        </div>
    </div>
    <JointCalibrationModal :index="index" :name="name" v-model:open="calibrationModalOpen" />
</template>

<script lang="ts" setup>
const remote = useRemote();

const props = defineProps<{
    index: number;
    name: string;
    range: number[];
}>();

const enabled = ref(false);
function toggleEnabled() {
    enabled.value = !enabled.value;
    sendEnabled(props.index, enabled.value);
}
async function sendEnabled(jointIndex: number, enabled: boolean) {
    await remote.setJointState(jointIndex, enabled);
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
        await remote.setJointTarget(props.index, newAngle);
    }, 100);
});

const calibrationState = ref<CalibrationState>('UNCALIBRATED');
function getCalibColor(state: CalibrationState) {
    return ({
        UNCALIBRATED: 'error',
        CALIBRATING: 'warning',
        CALIBRATED: 'success',
    }[state]) as 'error'|'warning'|'success';
}

const calibrationModalOpen = ref(false);
function onCalibrateClicked() {
    calibrationModalOpen.value = true;
}

const deleteCalibBtnLoading = ref(false);
async function onDeleteCalibrationClicked() {
    deleteCalibBtnLoading.value = true;
    try {
        await remote.deleteJointCalibration(props.index);
    } catch (err) {
        console.error("Error deleting calibration data :", err);
    }
    deleteCalibBtnLoading.value = false;
    calibrationState.value = await remote.getJointCalibrationState(props.index);
}

watch(calibrationModalOpen, async (newVal) => {
    calibrationState.value = await remote.getJointCalibrationState(props.index);
    enabled.value = await remote.getJointState(props.index);
});


let pollingInterval: number | null = null;
onMounted(async () => {
    enabled.value = await remote.getJointState(props.index);
    calibrationState.value = await remote.getJointCalibrationState(props.index);

    let counter = 0;
    pollingInterval = setInterval(async () => {
        modelAngle.value = Math.round(await remote.getJointModelAngle(props.index));
        feedbackAngle.value = Math.round(await remote.getJointFeedbackAngle(props.index));
        predictedAngle.value = Math.round(await remote.getJointPredictedAngle(props.index));
        if (counter++ === 5) // every 1s just to be sure it's up to date
        {
            enabled.value = await remote.getJointState(props.index);
            calibrationState.value = await remote.getJointCalibrationState(props.index);
            counter = 0;
        }
    }, 200);
});

onUnmounted(() => {
    if (pollingInterval) {
        clearInterval(pollingInterval);
    }
})
</script>

<style></style>