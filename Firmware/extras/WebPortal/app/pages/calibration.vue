<template>
    <div class="flex flex-col space-y-8 min-h-dvh min-w-full max-w-full max-h-dvh w-full h-dvh overflow-hidden p-4 lg:p-8">
        <div class="show-up space-y-2">
            <p class="text-center show-up text-4xl font-bold"> Calibration </p>
            <p class="text-center show-up text-lg text-gray-600 dark:text-gray-300"> Calibrate your TNY-360 for optimal performance. </p>
        </div>
        <div class="show-up delay-200 flex justify-center items-center w-full">
            <UButton class="w-fit" size="xl" variant="subtle" @click="startCalibration" label="Start Calibration" icon="lucide:play" :loading="startCalibrationButtonLoading" />
        </div>
        <div class="flex flex-col grow show-up delay-400 min-h-0 max-h-full h-full w-full max-w-lg items-center justify-center">
            <div ref="stepsContainer" class="min-h-0 max-h-full h-full w-full px-2 overflow-y-auto transition-all duration-500">
                <div class="flex flex-col gap-4 min-h-fit">
                    <button v-for="step of calibSteps" :key="step.id" :id="`step-${step.id}`" class="w-full border-2 rounded-lg p-2 transition-all" :class="cardColor(step.state)" @click="onStepClicked(step)">
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
                    <p class="text-center text-2xl font-semibold text-green-500"> Calibration Complete </p>
                    <p class="text-center text-lg text-gray-600 dark:text-gray-300"> Your TNY-360 is now calibrated! </p>
                    <p class="text-center text-lg text-gray-600 dark:text-gray-300"> You can now use your robot with full confidence after rebooting. </p>
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
import { JointId, MotorCalibrationState, MotorId } from '@tny-robotics/sdk';
import { DEG2RAD } from 'three/src/math/MathUtils';

const robot = useTNY360();

type DiagState = 'todo' | 'loading' | 'failed' | 'success';
type CalibStep = {
    id: string;
    title: string;
    state: DiagState;
    steps: {id: string, title: string}[];
    error: {code?: string, id?: number};
    jointId: JointId;
    fn_setup: () => Promise<void>;
    fn_start: () => Promise<void>;
    fn_progress: () => Promise<number>;
    fn_check: () => Promise<MotorCalibrationState>;
    fn_end: () => Promise<void>;
};

const createCalibStep = (id: string, title: string, jointId: JointId, startAngles: (number|undefined)[] = [], endAngles: (number|undefined)[] = []) => {
    return {
        id,
        title,
        state: 'todo' as DiagState,
        steps: [] as {id: string, title: string}[],
        error: {code: undefined, id: undefined},
        jointId,
        fn_setup: async () => {
            // set all joint angles
            for (let i = 0; i < startAngles.length; i++) {
                const angle = startAngles[i];
                if (angle !== undefined) {
                    await robot.value?.joint.setEnabled(i, true, true);
                    await robot.value?.joint.setAngle(i, angle * DEG2RAD, true);
                    // disable after a bit (to let enough time to reach the angle)
                    setTimeout(async () => await robot.value?.joint.setEnabled(i, false, true), 1000);
                }
            }
        },
        fn_start: async () => {
            await robot.value?.motor.startCalibration((jointId as unknown) as MotorId, true);
        },
        fn_progress: async () => {
            try {
                const progress = await robot.value?.motor.getCalibrationProgress((jointId as unknown) as MotorId);
                if (progress === undefined) throw new Error(`Calibration progress for joint ${jointId} is undefined`);
                return progress;
            } catch (error) {
                console.error(`Error getting calibration progress for joint ${jointId}:`, error);
                throw error;
            }
        },
        fn_check: async () => {
            try {
                const calibState = await robot.value?.motor.getCalibrationState((jointId as unknown) as MotorId);
                if (calibState === undefined) throw new Error(`Calibration state for joint ${jointId} is undefined`);
                return calibState;
            } catch (error) {
                console.error(`Error checking calibration state for joint ${jointId}:`, error);
                throw error;
            }
        },
        fn_end: async () => {
            // set all joint angles
            for (let i = 0; i < startAngles.length; i++) {
                const angle = startAngles[i];
                if (angle !== undefined) {
                    await robot.value?.joint.setEnabled(i, true, true);
                    await robot.value?.joint.setAngle(i, angle * DEG2RAD, true);
                    // disable after a bit (to let enough time to reach the angle)
                    setTimeout(async () => await robot.value?.joint.setEnabled(i, false, true), 1000);
                }
            }
        }
    };
};

const calibSteps = ref([
    createCalibStep('fl-hr', 'Front Left Hip Roll', JointId.FrontLeftHipRoll,
        [0, 40, 120, 0, -90, 120, 0, -90, 120, 0, -90, 120, 45, 45],
        [0, 40, 120, 0, -90, 120, 0, -90, 120, 0, -90, 120, 45, 45]
    ),
    createCalibStep('fl-hp', 'Front Left Hip Pitch', JointId.FrontLeftHipPitch,
        [0, -90, 120, 0, -90, 120, 0, -90, 120, 0, -90, 120],
        [0, -90, 120, 0, -90, 120, 0, -90, 120, 0, -90, 120]
    ),
    createCalibStep('fl-kp', 'Front Left Knee Pitch', JointId.FrontLeftKneePitch,
        [0, -45, 120, 0, -90, 120, 0, -90, 120, 0, -90, 120],
        [0, -45, 120, 0, -90, 120, 0, -90, 120, 0, -90, 120]
    ),
    createCalibStep('bl-hr', 'Back Left Hip Roll', JointId.BackLeftHipRoll,
        [0, 45, 120, 0, -130, 120, 0, -90, 120, 0, 45, 120],
        [0, 45, 120, 0, -130, 120, 0, -90, 120, 0, 45, 120]
    ),
    createCalibStep('bl-hp', 'Back Left Hip Pitch', JointId.BackLeftHipPitch,
        [0, 45, 120, 0, -90, 120, 0, -90, 120, 0, 45, 120],
        [0, 45, 120, 0, -90, 120, 0, -90, 120, 0, 45, 120]
    ),
    createCalibStep('bl-kp', 'Back Left Knee Pitch', JointId.BackLeftKneePitch,
        [0, 45, 120, 0, -45, 120, 0, -90, 120, 0, 45, 120],
        [0, 45, 120, 0, -45, 120, 0, -90, 120, 0, 45, 120]
    ),
    createCalibStep('br-hr', 'Back Right Hip Roll', JointId.BackRightHipRoll,
        [0, 45, 120, 0, -90, 120, 0, -130, 120, 0, 45, 120],
        [0, 45, 120, 0, -90, 120, 0, -130, 120, 0, 45, 120]
    ),
    createCalibStep('br-hp', 'Back Right Hip Pitch', JointId.BackRightHipPitch,
        [0, 45, 120, 0, -90, 120, 0, -90, 120, 0, 45, 120],
        [0, 45, 120, 0, -90, 120, 0, -90, 120, 0, 45, 120]
    ),
    createCalibStep('br-kp', 'Back Right Knee Pitch', JointId.BackRightKneePitch,
        [0, 45, 120, 0, -90, 120, 0, -45, 120, 0, 45, 120],
        [0, 45, 120, 0, -90, 120, 0, -45, 120, 0, 45, 120]
    ),
    createCalibStep('fr-hr', 'Front Right Hip Roll', JointId.FrontRightHipRoll,
        [0, 45, 120, 0, -90, 120, 0, -90, 120, 0, 40, 120],
        [0, 45, 120, 0, -90, 120, 0, -90, 120, 0, 40, 120]
    ),
    createCalibStep('fr-hp', 'Front Right Hip Pitch', JointId.FrontRightHipPitch,
        [0, 45, 120, 0, -90, 120, 0, -90, 120, 0, -90, 120],
        [0, 45, 120, 0, -90, 120, 0, -90, 120, 0, -90, 120]
    ),
    createCalibStep('fr-kp', 'Front Right Knee Pitch', JointId.FrontRightKneePitch,
        [0, 45, 120, 0, -90, 120, 0, -90, 120, 0, -45, 120],
        [0, 45, 120, 0, -90, 120, 0, -90, 120, 0, -45, 120]
    ),
] as CalibStep[]);

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

class CalibrationError extends Error {
    code: number;
    id: number;
    constructor(code: number, id: number) {
        super(`Calibration error: ${code} (id: ${id})`);
        this.code = code;
        this.id = id;
    }
}

const stepsContainer = ref<HTMLElement | null>(null);
const startCalibrationButtonLoading = ref(false);
async function startCalibration() {
    if (!stepsContainer.value) return;
    if (!stepsSuccess.value) return;
    stepsContainer.value.style.maxHeight = `${stepsContainer.value.scrollHeight}px`;
    stepsSuccess.value.style.maxHeight = `0px`;

    startCalibrationButtonLoading.value = true;
    for (let step of calibSteps.value) {
        if (step.state === 'success') continue;
        step.state = 'todo';
        step.steps = [];
    }

    for (let step of calibSteps.value) {
        if (step.state === 'success') continue;
        step.state = 'loading';
        focusStep(step.id);
        await new Promise((resolve) => setTimeout(resolve, 100));

        // Start the calibration
        try {
            // setup for joint calibration
            step.steps.push({id: 'setup', title: 'Setting up ...'});
            focusStep(step.id);
            await step.fn_setup();

            // trigger the actual calibration
            step.steps.push({id: 'start', title: 'Starting calibration ...'});
            focusStep(step.id);
            await step.fn_start();

            // loop while calibration is in progress
            let finished = false;
            step.steps.push({id: 'progress', title: 'Progresss: 0%'});
            focusStep(step.id);
            while (!finished) {
                const progress = await step.fn_progress();
                const progressStep = step.steps.find(s => s.id === 'progress');
                if (progressStep) progressStep.title = `Progress: ${Math.round(progress * 100)}%`;
                else console.warn(`Progress step not found for step ${step.id}`);
                const calibState = await step.fn_check();
                if (calibState === MotorCalibrationState.Calibrating) finished = false;
                else if (calibState === MotorCalibrationState.Calibrated) finished = true;
                else {
                    const lastErrorEvent = await robot.value?.error.getErrorEventByIndex(0);
                    if (!lastErrorEvent) throw new CalibrationError(-1, -1);
                    throw new CalibrationError(lastErrorEvent.fullCode, lastErrorEvent.eventId);
                }
            }

            // now clean up for the next step
            step.steps.push({id: 'end', title: 'Finishing ...'});
            focusStep(step.id);
            await step.fn_end();
        } catch (error: any) {
            step.state = 'failed';
            step.error.code = error?.code ?? 'UNKNOWN_ERROR';
            step.error.id = error?.id ?? undefined;
            console.error(`Error in step ${step.id}:`, error);
            step.steps.push({id: 'failed', title: error?.message ?? 'Calibration failed!'});
            focusStep(step.id);
            startCalibrationButtonLoading.value = false;
            return;
        }

        step.steps.push({id: 'done', title: 'Done'});
        focusStep(step.id);
        await new Promise((resolve) => setTimeout(resolve, 100));
        step.state = 'success';
        await new Promise((resolve) => setTimeout(resolve, 100));
    }

    startCalibrationButtonLoading.value = false;
    onCalibrationFinished();
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
async function onCalibrationFinished() {
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

async function checkCalibrationStates() {
    console.log('Checking calibration states...');
    for (let step of calibSteps.value) {
        const calibState = await step.fn_check();
        if (calibState === MotorCalibrationState.Calibrated) {
            step.state = 'success';
        }
    }
}

watch(robot, async (tny) => {
    if (tny) {
        tny.on('connected', async () => {
            await checkCalibrationStates();
        });
    }
}, { immediate: true });

onMounted(async () => {
    if (robot.value) await checkCalibrationStates();
});

</script>

<style></style>