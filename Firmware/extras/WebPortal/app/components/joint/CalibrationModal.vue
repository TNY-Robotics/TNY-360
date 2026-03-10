<template>
    <UModal :title="`Calibration - ${name}`" v-model:open="choiceModalOpen">
        <template #body>
            <div class="flex w-full h-full space-x-8">
                <button class="calib-choice-btn flex flex-col space-y-4 text-center" @click="manualModalOpen = true">
                    <div class="flex space-x-2 items-center">
                        <UIcon name="i-lucide-settings-2" class="w-8 h-8" />
                        <p class="text-xl font-semibold"> Manual </p>
                    </div>
                    <p>
                        Tweak each joint settings manually.
                    </p>
                </button>
                <button class="calib-choice-btn flex flex-col space-y-4 text-center" @click="automaticModalOpen = true">
                    <div class="flex space-x-2 items-center">
                        <UIcon name="i-lucide-bot" class="w-8 h-8" />
                        <p class="text-xl font-semibold"> Automatic </p>
                    </div>
                    <p>
                        Let your TNY-360 do the calibration.
                    </p>
                </button>
            </div>
        </template>
    </UModal>
    <UModal :title="`Manual Calibration - ${name}`" v-model:open="manualModalOpen" :close="false" :dismissible="false">
        <template #body>
            <div class="space-y-2">
                <div class="flex space-x-4 justify-between w-full items-center">
                    <p> min_pwm </p>
                    <UInput class="max-w-48" type="number" v-model="manualModalData.calib_data.min_pwm" />
                </div>
                <div class="flex space-x-4 justify-between w-full items-center">
                    <p> max_pwm </p>
                    <UInput class="max-w-48" type="number" v-model="manualModalData.calib_data.max_pwm" />
                </div>
                <div class="flex space-x-4 justify-between w-full items-center">
                    <p> min_voltage </p>
                    <UInput class="max-w-48" type="number" v-model="manualModalData.calib_data.min_voltage" />
                </div>
                <div class="flex space-x-4 justify-between w-full items-center">
                    <p> max_voltage </p>
                    <UInput class="max-w-48" type="number" v-model="manualModalData.calib_data.max_voltage" />
                </div>
                <div class="flex space-x-4 justify-between w-full items-center">
                    <p> feedback_noise </p>
                    <UInput class="max-w-48" type="number" v-model="manualModalData.calib_data.feedback_noise" />
                </div>
                <div class="flex space-x-4 justify-between w-full items-center">
                    <p> pwm_deadband </p>
                    <UInput class="max-w-48" type="number" v-model="manualModalData.calib_data.pwm_deadband" />
                </div>
                <div class="flex space-x-4 justify-between w-full items-center">
                    <p> feedback_latency_ms </p>
                    <UInput class="max-w-48" type="number" v-model="manualModalData.calib_data.feedback_latency_ms" />
                </div>
            </div>
        </template>
        <template #footer>
            <div class="flex w-full justify-between items-center">
                <UButton label="Cancel" variant="ghost" color="neutral" icon="i-lucide-x" @click="onManualModalCancelClicked" />
                <UButton :loading="manualModalSaveLoading" label="Save" variant="soft" color="primary" icon="i-lucide-check" trailing @click="onManualModalSaveClicked" />
            </div>
        </template>
    </UModal>
    <UModal :title="`Automatic Calibration - ${name}`" v-model:open="automaticModalOpen" :close="false" :dismissible="false">
        <template #body>
            <div class="space-y-4">
                <div class="flex justify-between w-full">
                    <h2 class="text-xl font-semibold"> {{ autoModalData.topMessage }} </h2>
                    <h2 class="text-xl font-semibold"> {{ autoModalData.topProgress }} </h2>
                </div>
                <span class="flex w-full h-2 rounded-full bg-slate-200 dark:bg-slate-700">
                    <span ref="autoModalProgress" class="flex h-full rounded-full bg-primary-500 dark:bg-primary-500"
                        style="width: 0%; transition: width 0.5s cubic-bezier(0.1, .8, 0.2, 1)"/>
                </span>
                <div class="p-2">
                    <p> {{ autoModalData.description }} </p>
                </div>
            </div>
        </template>
        <template #footer>
            <div class="flex w-full justify-between items-center">
                <UButton :disabled="autoModalCancelDisabled" label="Cancel" variant="ghost" color="neutral" icon="i-lucide-x" @click="onAutoModalCancelClicked" />
                <UButton :disabled="autoModalDoneDisabled" label="Done" variant="soft" color="primary" icon="i-lucide-check" trailing @click="onAutoModalDoneClicked" />
            </div>
        </template>
    </UModal>
</template>

<script setup lang="ts">
const remote = useRemote();

const props = defineProps<{
    index: number,
    name: string
}>();

const modelOpen = defineModel<boolean>('open');

const choiceModalOpen = ref(false);
const manualModalOpen = ref(false);
const automaticModalOpen = ref(false);

watch(modelOpen, (newval) => {
    choiceModalOpen.value = newval as boolean;
    manualModalOpen.value = false;
    automaticModalOpen.value = false;
});
watch(choiceModalOpen, (newVal) => {
    if (!manualModalOpen.value && !automaticModalOpen.value && !newVal) {
        modelOpen.value = false;
    }
});

function onModalChanged(newVal: boolean) {
    if (!newVal) {
        choiceModalOpen.value = false;
        manualModalOpen.value = false;
        automaticModalOpen.value = false;
        modelOpen.value = false;
    } else {
        choiceModalOpen.value = false;
    }
}
watch(manualModalOpen, onModalChanged);
watch(automaticModalOpen, onModalChanged);

watch(manualModalOpen, (newVal) => { if (newVal) onManualCalibrationNModalOpened() });
watch(automaticModalOpen, (newVal) => { if (newVal) onAutomaticCalibrationNModalOpened() });

/// MANUAL CALIBRATION MODAL ///
const manualModalData = reactive({
    calib_data: {
        min_pwm: 0,
        max_pwm: 0,
        min_voltage: 0,
        max_voltage: 0,
        feedback_noise: 0,
        pwm_deadband: 0,
        feedback_latency_ms: 0,
    } as CalibrationData
});
const manualModalSaveLoading = ref(false);
async function onManualCalibrationNModalOpened() {
    manualModalData.calib_data = await remote.getJointCalibrationData(props.index);
}
async function onManualModalCancelClicked() {
    manualModalOpen.value = false;
}
async function onManualModalSaveClicked() {
    manualModalSaveLoading.value = true;
    try {
        await remote.setJointCalibrationData(props.index, manualModalData.calib_data);
    } catch (err) {
        console.error("Error saving joint calibration data : ", err);
    }
    manualModalSaveLoading.value = false;
    await new Promise((resolve) => setTimeout(resolve, 200));
    manualModalOpen.value = false;
}

/// AUTOMATIC CALIBRATION MODAL ///

const autoModalData = reactive({
    topMessage: '',
    topProgress: '',
    description: '',
});
const autoModalProgress = ref<HTMLSpanElement|null>(null);
const autoModalDoneDisabled = ref(true);
const autoModalCancelDisabled = ref(true);
let autoModalCalibPollingInterval: number | null = null;
async function onAutomaticCalibrationNModalOpened() {
    autoModalDoneDisabled.value = true;
    autoModalCancelDisabled.value = false;
    autoModalData.topMessage = 'Starting Automatic Calibration ...';
    autoModalData.topProgress = '';
    autoModalData.description = '';

    try {
        await remote.startJointCalibration(props.index);
        autoModalCalibPollingInterval = setInterval(() => {
            fetchAutoCalibProgress();
        }, 500);
    } catch (err) {
        console.error("Error start joint calibration :", err);
    }
}
async function fetchAutoCalibProgress() {
    const progress = await remote.getJointCalibrationProgress(props.index);
    const status = await remote.getJointCalibrationState(props.index);

    autoModalData.topMessage = 'Calibrating ...';
    autoModalData.description = 'Joint calibration can take up to 2 minutes.';
    autoModalData.topProgress = `${Math.round(progress*100)}%`;
    if (autoModalProgress.value) autoModalProgress.value.style.width = `${Math.round(progress*100)}%`;

    if (status === 'UNCALIBRATED') { // error
        autoModalData.topMessage = 'Something went wrong :/';
        autoModalData.description = 'The calibration failed. Please refer to the documentation for troubleshooting.';
        autoModalData.topProgress = '';
    }
    
    if (status === 'CALIBRATED') { // success !
        autoModalData.topMessage = 'Calibration complete!';
        autoModalData.description = 'All done!';
        autoModalData.topProgress = '100%';
        if (autoModalCalibPollingInterval) { // clear interval and enable/disable buttons
            clearInterval(autoModalCalibPollingInterval);
            autoModalDoneDisabled.value = false;
            autoModalCancelDisabled.value = true;
        }
    }
}
async function onAutoModalCancelClicked() {
    await remote.stopJointCalibration(props.index);    
    automaticModalOpen.value = false;
}
function onAutoModalDoneClicked() { // already saved at the end, just close
    automaticModalOpen.value = false;
}

</script>

<style scoped>
@import 'tailwindcss';

.calib-choice-btn {
    @apply flex justify-center items-center w-1/2 h-full aspect-square p-4
           border-dashed border-2 border-slate-400 dark:border-slate-500 hover:bg-slate-50 hover:dark:bg-slate-800;
    border-radius: 1em;
}
</style>