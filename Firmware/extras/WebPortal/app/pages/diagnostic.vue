<template>
    <div class="flex flex-col space-y-8 min-h-dvh min-w-full max-w-full max-h-dvh w-full h-dvh overflow-hidden p-4 lg:p-8">
        <div class="show-up space-y-2">
            <p class="text-center show-up text-4xl font-bold"> Diagnostic </p>
            <p class="text-center show-up text-lg text-gray-600 dark:text-gray-300"> Let's see if your TNY-360 is in good health! </p>
        </div>
        <div class="show-up delay-200 flex justify-center items-center w-full">
            <UButton class="w-fit" size="xl" variant="subtle" @click="startDiagnostic" label="Start Diagnostic" icon="lucide:play" :loading="startDiagnosticButtonLoading" />
        </div>
        <div class="flex flex-col grow show-up delay-400 min-h-0 max-h-full h-full w-full max-w-lg items-center justify-center">
            <div ref="stepsContainer" class="min-h-0 max-h-full h-full w-full px-2 overflow-y-auto transition-all duration-500">
                <div class="flex flex-col gap-4 min-h-fit">
                    <button v-for="step of diagSteps" :key="step.id" :id="`step-${step.id}`" class="w-full border-2 rounded-lg p-2 transition-all" :class="cardColor(step.state)" @click="onStepClicked(step)">
                        <div class="flex space-x-8 justify-between w-full">
                            <p class="p-2 text-lg transition-all" :class="step.state === 'todo' ? 'font-normal' : 'font-semibold'">
                                {{ step.title }}
                            </p>
                            <div class="flex justify-start items-center">
                                <UIcon :name="stateIcon(step.state)" class="m-0 p-0 w-6 h-6" :class="{
                                    'animate-spin mr-2': step.state === 'loading',
                                    'text-red-500': step.state === 'failed',
                                    'text-green-500 mr-2': step.state === 'success',
                                }" />
                                <UIcon v-if="step.state === 'failed'" name="lucide:chevron-right" class="show-left m-0 p-0 w-8 h-8" />
                            </div>
                        </div>
                        <div v-if="step.state === 'loading' || step.state === 'failed'" class="p-2 pt-0">
                            <div class="flex flex-col justify-start items-start w-full">
                                <p v-for="substep in step.steps" :key="substep.id" class="font-mono text-sm opacity-60">
                                    > {{ substep.title }}
                                </p>
                            </div>
                        </div>
                    </button>
                </div>
            </div>
            <div ref="stepsSuccess" class="max-h-0 max-w-full overflow-hidden transition-all duration-500">
                <div class="flex flex-col gap-4 min-h-fit">
                    <UIcon name="lucide:check" class="w-16 h-16 text-green-500 mx-auto" />
                    <p class="text-center text-2xl font-semibold text-green-500"> Diagnostic Complete </p>
                    <p class="text-center text-lg text-gray-600 dark:text-gray-300"> Your TNY-360 is rock solid! </p>
                    <p class="text-center text-lg text-gray-600 dark:text-gray-300"> Reboot your robot to continue </p>
                    <br />
                    <UButton class="w-fit mx-auto" size="xl" variant="subtle" @click="rebootRobot" label="Reboot" icon="lucide:rotate-ccw" /> 
                </div>
            </div>
        </div>
    </div>
    <UModal v-model:open="stepErrorModalOpen" :title="`Error ${modalStep?.error.code ?? ''}`" :closable="true" class="show-left" :fullscreen="true">
        <template #body>
            <ErrorEventCard :code="modalStep?.error.code" :id="modalStep?.error.id" />
        </template>
    </UModal>
</template>

<script lang="ts" setup>
const robot = useTNY360();

type DiagState = 'todo' | 'loading' | 'failed' | 'success';

const createDiagStep = (id: string, title: string, fn: () => Promise<number>) => ({
    id,
    title,
    state: 'todo' as DiagState,
    steps: [] as {id: string, title: string}[], // precision on the subtest
    error: {
        id: undefined as undefined|number, // event id
        code: undefined as undefined|number // error code
    },
    fn
});

const diagSteps = ref([
    createDiagStep('led', 'LED System', async () => robot.value?.diagnostic.checkLED() ?? 0),
    createDiagStep('i2c', 'I2C Bus', async () => robot.value?.diagnostic.checkI2C() ?? 0),
    createDiagStep('speaker', 'Speaker', async () => robot.value?.diagnostic.checkSpeaker() ?? 0),
    createDiagStep('microphone', 'Microphone', async () => robot.value?.diagnostic.checkMicrophone() ?? 0),
    createDiagStep('camera', 'Camera', async () => robot.value?.diagnostic.checkCamera() ?? 0),
    createDiagStep('laser', 'Laser sensor', async () => robot.value?.diagnostic.checkLaser() ?? 0),
    createDiagStep('imu', 'Inertial Measurement Unit', async () => robot.value?.diagnostic.checkIMU() ?? 0),
    createDiagStep('power', 'Power Management', async () => robot.value?.diagnostic.checkPower() ?? 0),
    createDiagStep('motordriver', 'Motor Driver', async () => robot.value?.diagnostic.checkMotorDriver() ?? 0),
    createDiagStep('analogreader', 'Analog Reader', async () => robot.value?.diagnostic.checkAnalogReader() ?? 0),
    createDiagStep('controlloop', 'Control Loop', async () => robot.value?.diagnostic.checkControlLoop() ?? 0),
    createDiagStep('decisionloop', 'Decision Loop', async () => robot.value?.diagnostic.checkDecisionLoop() ?? 0),
]);

function focusStep(stepId: string) {
    const div = document.getElementById(`step-${stepId}`);
    if (div) {
        if (stepsContainer.value) {
            const containerRect = stepsContainer.value.getBoundingClientRect();
            const divRect = div.getBoundingClientRect();
            const distance = divRect.top - containerRect.top + stepsContainer.value.scrollTop;
            // if we scroll past the maximum scroll point, do it instant, otherwise, scroll smoothly
            if (distance > stepsContainer.value.scrollHeight - stepsContainer.value.clientHeight) {
                nextTick(() => {
                    if (!stepsContainer.value) return;
                    stepsContainer.value.scrollTo({
                        top: stepsContainer.value.scrollHeight - stepsContainer.value.clientHeight,
                        behavior: 'auto'
                    });
                });
            } else {
                stepsContainer.value.scrollTo({
                    top: distance,
                    behavior: 'smooth'
                });
            }
        }
    }
}

const stepsContainer = ref<HTMLElement | null>(null);
const startDiagnosticButtonLoading = ref(false);
async function startDiagnostic() {
    if (!stepsContainer.value) return;
    if (!stepsSuccess.value) return;
    stepsContainer.value.style.maxHeight = `${stepsContainer.value.scrollHeight}px`;
    stepsSuccess.value.style.maxHeight = `0px`;

    startDiagnosticButtonLoading.value = true;
    for (let step of diagSteps.value) {
        step.state = 'todo';
        step.steps = [];
    }

    for (let step of diagSteps.value) {
        step.state = 'loading';
        focusStep(step.id);
        await new Promise((resolve) => setTimeout(resolve, 100));
        step.steps.push({id: 'init', title: 'Starting test ...'});
        focusStep(step.id);

        // Run actual test
        try {
            const res = await step.fn();
            if (res === 0) {
                step.steps.push({id: 'success', title: 'Test passed!'});
                focusStep(step.id);
            } else {
                step.state = 'failed';
                step.error.code = undefined;
                step.error.id = res; // event id
                step.steps.push({id: 'failed', title: `Error ID 0x${res.toString(16).toUpperCase()}`});
                focusStep(step.id);
                startDiagnosticButtonLoading.value = false;
                return;
            }
        } catch (error: any) {
            step.state = 'failed';
            step.error.code = error?.code ?? 'UNKNOWN_ERROR';
            step.error.id = error?.id ?? undefined;
            console.error(`Error in step ${step.id}:`, error);
            step.steps.push({id: 'failed', title: error?.message ?? 'Test failed!'});
            focusStep(step.id);
            startDiagnosticButtonLoading.value = false;
            return;
        }

        step.steps.push({id: 'done', title: 'Done'});
        focusStep(step.id);
        await new Promise((resolve) => setTimeout(resolve, 100));
        step.state = 'success';
        await new Promise((resolve) => setTimeout(resolve, 100));
    }

    startDiagnosticButtonLoading.value = false;
    onAllTestsPassed();
}

function stateIcon(state: DiagState) {
    switch (state) {
        case 'loading': return 'lucide:loader-circle';
        case 'failed': return 'lucide:x';
        case 'success': return 'lucide:check';
        default: return '';
    }
}

function cardColor(state: DiagState) {
    switch (state) {
        case 'loading': return 'bg-yellow-500/10 dark:bg-yellow-500/10 border-yellow-500 dark:border-yellow-500';
        case 'failed': return 'bg-red-500/10 dark:bg-red-500/10 border-red-500 dark:border-red-500';
        case 'success': return 'bg-green-500/10 dark:bg-green-500/10 border-green-500 dark:border-green-500';
        default: return 'bg-slate-200/10 dark:bg-slate-700/10 border-slate-200 dark:border-slate-700';
    }
}

const stepsSuccess = ref<HTMLElement | null>(null);
async function onAllTestsPassed() {
    if (!stepsContainer.value) return;
    if (!stepsSuccess.value) return;
    
    const rect = stepsContainer.value.getBoundingClientRect();
    stepsContainer.value.style.maxHeight = `${rect.height}px`;
    stepsSuccess.value.style.maxHeight = `0px`;
    
    await robot.value?.diagnostic.setDiagnosticMode(false);

    stepsContainer.value.style.maxHeight = `0px`;
    const rect2 = stepsSuccess.value.firstElementChild?.getBoundingClientRect();
    if (!rect2) return;
    stepsSuccess.value.style.maxHeight = `${rect2.height}px`;
}

const stepErrorModalOpen = ref(false);
const modalStep = ref<any>(null);
async function onStepClicked(step: any) {
    if (step.state !== 'failed') return;
    modalStep.value = step;
    stepErrorModalOpen.value = true;
}

async function rebootRobot() {
    await robot.value?.system.reboot();
}

</script>

<style></style>