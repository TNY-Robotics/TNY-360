<template>
    <UModal :title="`Calibration - ${name}`" v-model:open="choiceModalOpen">
        <template #body>
            <div class="flex w-full h-full space-x-8">
                <button class="calib-choice-btn flex flex-col space-y-4 text-center" @click="manualModalOpen = true">
                    <div class="flex space-x-2 items-center">
                        <UIcon name="lucide:settings-2" class="w-8 h-8" />
                        <p class="text-xl font-semibold"> Manual </p>
                    </div>
                    <p>
                        Tweak each joint settings manually.
                    </p>
                </button>
                <button class="calib-choice-btn flex flex-col space-y-4 text-center" @click="automaticModalOpen = true">
                    <div class="flex space-x-2 items-center">
                        <UIcon name="lucide:bot" class="w-8 h-8" />
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
            <div v-if="manualModalData.calib_data === undefined" class="flex flex-col space-y-2 grow justify-center items-center">
                <p class="text-xl font-semibold"> Loading ... </p>
                <p> Loading calibration data </p>
            </div>
            <div v-else-if="manualModalData.calib_data === null" class="flex flex-col space-y-2 grow justify-center items-center">
                <p class="text-xl font-semibold"> Failed to load calibration data :/ </p>
                <p> Please try again later. </p>
            </div>
            <div v-else class="space-y-2">
                <div class="flex space-x-4 justify-between w-full items-center">
                    <p> Min. Duty Cycle </p>
                    <UInput class="max-w-48" type="number" v-model="manualModalData.calib_data.dc_min" />
                </div>
                <div class="flex space-x-4 justify-between w-full items-center">
                    <p> Max. Duty Cycle </p>
                    <UInput class="max-w-48" type="number" v-model="manualModalData.calib_data.dc_max" />
                </div>
                <div class="flex space-x-4 justify-between w-full items-center">
                    <p> Duty Cycle Deadband </p>
                    <UInput class="max-w-48" type="number" v-model="manualModalData.calib_data.dc_deadband" />
                </div>
                <div class="flex space-x-4 justify-between w-full items-center">
                    <p> Min. Feedback Voltage (V) </p>
                    <UInput class="max-w-48" type="number" v-model="manualModalData.calib_data.feedback_min" />
                </div>
                <div class="flex space-x-4 justify-between w-full items-center">
                    <p> Max. Feedback Voltage (V) </p>
                    <UInput class="max-w-48" type="number" v-model="manualModalData.calib_data.feedback_max" />
                </div>
                <div class="flex space-x-4 justify-between w-full items-center">
                    <p> Feedback Noise (V) </p>
                    <UInput class="max-w-48" type="number" v-model="manualModalData.calib_data.feedback_noise" />
                </div>
                <div class="flex space-x-4 justify-between w-full items-center">
                    <p> Feedback Latency (ms) </p>
                    <UInput class="max-w-48" type="number" v-model="manualModalData.calib_data.feedback_latency_ms" />
                </div>
                <div class="flex space-x-4 justify-between w-full items-center">
                    <p> Feedback Inverted </p>
                    <USwitch v-model="manualModalData.calib_data.feedback_inverted" />
                </div>
                <div class="flex space-x-4 justify-between w-full items-center">
                    <p> Maximum Speed (range/s) </p>
                    <UInput class="max-w-48" type="number" v-model="manualModalData.calib_data.max_speed" />
                </div>
            </div>
        </template>
        <template #footer>
            <div class="flex w-full justify-between items-center">
                <UButton label="Cancel" variant="ghost" color="neutral" icon="lucide:x" @click="onManualModalCancelClicked" />
                <UButton :loading="manualModalSaveLoading" label="Save" variant="soft" color="primary" icon="lucide:check" trailing @click="onManualModalSaveClicked" />
            </div>
        </template>
    </UModal>
    <UModal :title="`Automatic Calibration - ${name}`" v-model:open="automaticModalOpen" :close="false" :dismissible="false">
        <template #body>
            <div class="space-y-4">
                <div class="flex justify-between w-full">
                    <h3> {{ autoModalData.topMessage }} </h3>
                    <p> {{ autoModalData.topProgress }} </p>
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
                <UButton :disabled="autoModalCancelDisabled" label="Cancel" variant="ghost" color="neutral" icon="lucide:x" @click="onAutoModalCancelClicked" />
                <UButton :disabled="autoModalDoneDisabled" label="Done" variant="soft" color="primary" icon="lucide:check" trailing @click="onAutoModalDoneClicked" />
            </div>
        </template>
    </UModal>
</template>

<script setup lang="ts">
import { MotorCalibrationState, type MotorCalibrationData } from '@tny-robotics/sdk';
import { toRaw } from 'vue';

const tny = useTNY360();

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

watch(manualModalOpen, (newVal) => { if (newVal) onManualCalibrationModalOpened() });
watch(automaticModalOpen, (newVal) => { if (newVal) onAutomaticCalibrationModalOpened() });

/// MANUAL CALIBRATION MODAL ///
const manualModalData = reactive({
    calib_data: undefined as MotorCalibrationData|undefined|null
});
const manualModalSaveLoading = ref(false);
async function onManualCalibrationModalOpened() {
    manualModalData.calib_data = undefined;
    try {
        manualModalData.calib_data = await tny.value?.motor.getCalibrationData(props.index);
        // little fix for long float values : round to 4 decimals
        for (const key in manualModalData.calib_data) {
            if (typeof manualModalData.calib_data[key as keyof MotorCalibrationData] === 'number') {
                (manualModalData.calib_data as any)[key as keyof MotorCalibrationData] = Math.round((manualModalData.calib_data[key as keyof MotorCalibrationData] as number) * 10000) / 10000;
            }
        }
        console.log("Calibration data : ", toRaw(manualModalData.calib_data));
    } catch (err) {
        console.error("Error fetching joint calibration data : ", err);
        manualModalData.calib_data = null;
    }
}
async function onManualModalCancelClicked() {
    manualModalOpen.value = false;
}
async function onManualModalSaveClicked() {
    if (!manualModalData.calib_data) return;

    manualModalSaveLoading.value = true;
    try {
        await tny.value?.motor.setCalibrationData(props.index, toRaw(manualModalData.calib_data), true);
        console.log("Calibration data saved : ", toRaw(manualModalData.calib_data));
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
async function onAutomaticCalibrationModalOpened() {
    autoModalDoneDisabled.value = true;
    autoModalCancelDisabled.value = false;
    autoModalData.topMessage = 'Starting Automatic Calibration ...';
    autoModalData.topProgress = '';
    autoModalData.description = '';

    try {
        await tny.value?.motor.startCalibration(props.index, true);
        autoModalCalibPollingInterval = setInterval(() => {
            fetchAutoCalibProgress();
        }, 500);
    } catch (err) {
        console.error("Error start joint calibration :", err);
    }
}
async function fetchAutoCalibProgress() {
    const progress = await tny.value?.motor.getCalibrationProgress(props.index) || 0;
    const status = await tny.value?.motor.getCalibrationState(props.index);

    autoModalData.topMessage = 'Calibrating ...';
    autoModalData.description = 'Joint calibration can take up to 2 minutes.';
    autoModalData.topProgress = `${Math.round(progress*100)}%`;
    if (autoModalProgress.value) autoModalProgress.value.style.width = `${Math.round(progress*100)}%`;

    if (status === MotorCalibrationState.Error) { // error
        autoModalData.topMessage = 'Something went wrong :/';
        autoModalData.description = 'The calibration failed. Please refer to the documentation for troubleshooting.';
        autoModalData.topProgress = '';
    }
    
    if (status === MotorCalibrationState.Calibrated) { // success !
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
    await tny.value?.motor.stopCalibration(props.index);
    automaticModalOpen.value = false;
}
function onAutoModalDoneClicked() { // already saved at the end, just close
    automaticModalOpen.value = false;
}

</script>

<style scoped>
@import 'tailwindcss';

.calib-choice-btn {
    @apply flex justify-center items-center w-1/2 h-full aspect-square p-4 border-dashed border-2
        border-slate-400 dark:border-slate-500 hover:bg-slate-50 hover:dark:bg-slate-800;
    border-radius: 1em;
}
.calib-choice-btn-disabled {
    @apply flex justify-center items-center w-1/2 h-full aspect-square p-4 border-dashed border-2
        border-slate-300 dark:border-slate-600 text-slate-400 dark:text-slate-600 ;
    border-radius: 1em;
}
</style>