<template>
    <div class="flex flex-col min-h-dvh min-w-full max-w-full max-h-dvh w-full h-dvh overflow-hidden p-4 lg:p-8">
        <p class="text-center show-up text-6xl font-extrabold"> TNY - 360 </p>
        <p class="text-center show-up text-lg text-gray-600 dark:text-gray-300"> Welcome on the TNY-360 Web Portal!</p>
        <div class="flex grow justify-center items-center">
            <div class="flex flex-col space-y-8 h-fit w-fit justify-center items-center">
                <UButton class="show-up delay-200 flex justify-between w-fit" size="xl" variant="subtle" href="/teleop" label="Jouer avec mon TNY-360" icon="lucide:gamepad-2" />
                <UButton class="show-up delay-400 flex justify-between w-fit" size="xl" variant="subtle" href="/dashboard" label="Programmer mon TNY-360" icon="lucide:chart-no-axes-gantt" disabled color="neutral" />
                <UButton class="show-up delay-600 flex justify-between w-fit" size="xl" variant="subtle" href="/dashboard" label="Voir le Panneau de Contrôle" icon="lucide:gauge" />
            </div>
        </div>
        <div class="flex w-full justify-between">
            <UButton variant="soft" label="Diagnostic Mode" icon="lucide:wrench" @click="onDiagModeClicked" />
        </div>
    </div>
    <UModal v-model:open="calibrationPopupOpen" title="Not Calibrated">
        <template #body>
            <div class="space-y-4">
                <div class="space-y-1">
                    <p> We detected that your robot is not yet calibrated. </p>
                    <p> Without calibration, your TNY-360 won't have position feedback, automatic misalignment correction, and may not walk and move properly. </p>
                </div>
                <p> Do you want to calibrate your robot now? </p>
            </div>
        </template>
        <template #footer>
            <div class="flex justify-between w-full space-x-4">
                <UButton variant="link" color="neutral" label="Skip" icon="lucide:x" @click="() => { calibrationPopupOpen = false }" />
                <UButton variant="solid" color="primary" label="Calibrate" icon="lucide:arrow-right" trailing href="/calibration" />
            </div>
        </template>
    </UModal>
    <UModal v-model:open="diagnosticPopupOpen" title="Diagnostic Mode">
        <template #body>
            <div class="space-y-4">
                <div class="space-y-1">
                    <p> Your robot will need to reboot into diagnostic mode before you can start the diagnostic. </p>
                </div>
                <p> Do you want to reboot into diagnostic mode? </p>
            </div>
        </template>
        <template #footer>
            <div class="flex justify-between w-full space-x-4">
                <UButton variant="link" color="neutral" label="Cancel" @click="() => { diagnosticPopupOpen = false }" />
                <UButton variant="solid" color="primary" label="Reboot" icon="lucide:arrow-right" trailing @click="onDiagModeReboot" :loading="diagModeBtnLoading" />
            </div>
        </template>
    </UModal>
</template>

<script lang="ts" setup>
import { MotorCalibrationState, MotorId } from '@tny-robotics/sdk';

const robot = useTNY360();
const router = useRouter();

const calibrationPopupOpen = ref(false);

watch(robot, async (tny) => {
    if (!tny) return;
    tny.on('connected', async () => {
        // check for diagnostic mode
        const diagEnabled = await tny.diagnostic.isEnabled();
        if (diagEnabled) {
            router.replace('diagnostic');
            return;
        }
        // check for uncalibrated state
        const promises = Array.from({ length: 12 }, (_, i) => tny.motor.getCalibrationState(i as MotorId));
        const calibStates = await Promise.all(promises);
        const hasUncalibrated = calibStates.some(state => state !== MotorCalibrationState.Calibrated);
        if (hasUncalibrated) calibrationPopupOpen.value = true;
    });
}, { immediate: true });

const diagnosticPopupOpen = ref(false);
const diagModeBtnLoading = ref(false);
function onDiagModeClicked() {
    diagnosticPopupOpen.value = true;
    diagModeBtnLoading.value = false;
}

async function onDiagModeReboot() {
    diagModeBtnLoading.value = true;
    robot.value?.diagnostic.rebootInDiagnosticMode();
    await new Promise((resolve) => setTimeout(resolve, 1000));
    diagnosticPopupOpen.value = false;
    window.close();
}

</script>

<style></style>