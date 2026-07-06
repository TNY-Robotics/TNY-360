<template>
    <div class="p-4 space-y-4">
        <h2>Loop Control</h2>
        <div class="show-right space-y-8">
            <div class="flex space-x-4">
                <p class="text-lg font-semibold">Control Loop</p>
                <UButton :label="controlLoopEnabled ? 'Enabled' : 'Disabled'" :icon="controlLoopEnabled? 'lucide:check': 'lucide:x'" :color="controlLoopLoading? 'neutral' : (controlLoopEnabled? 'success' : 'error')"
                    @click="onControlLoopToggle" :loading="controlLoopLoading" variant="subtle" trailing />
            </div>
            <div class="flex space-x-4">
                <p class="text-lg font-semibold">Decision Loop</p>
                <UButton :label="decisionLoopEnabled ? 'Enabled' : 'Disabled'" :icon="decisionLoopEnabled? 'lucide:check': 'lucide:x'" :color="decisionLoopLoading? 'neutral' : (decisionLoopEnabled? 'success' : 'error')"
                    @click="onDecisionLoopToggle" :loading="decisionLoopLoading" variant="subtle" trailing />
            </div>
        </div>
        <h2>ADC Voltages</h2>
        <div class="show-up space-y-8">
            <div class="pl-4">
                <h3 class="text-xl font-bold"> Front-Left Leg </h3>
                <div class="flex flex-col lg:flex-row space-x-4">
                    <div class="w-full lg:w-1/4 p-2 space-y-2">
                        <p> Channel 1 (Hip Roll) </p>
                        <ValueChart :values="adcVoltages[0] ?? []" :range="[0, 3.3]" class="h-48" />
                    </div>
                    <div class="w-full lg:w-1/4 p-2 space-y-2">
                        <p> Channel 2 (Hip Pitch) </p>
                        <ValueChart :values="adcVoltages[1] ?? []" :range="[0, 3.3]" class="h-48" />
                    </div>
                    <div class="w-full lg:w-1/4 p-2 space-y-2">
                        <p> Channel 3 (Knee Pitch) </p>
                        <ValueChart :values="adcVoltages[2] ?? []" :range="[0, 3.3]" class="h-48" />
                    </div>
                    <div class="w-full lg:w-1/4 p-2 space-y-2">
                        <p> Channel 4 (Feet Contact) </p>
                        <ValueChart :values="adcVoltages[3] ?? []" :range="[0, 3.3]" class="h-48" />
                    </div>
                </div>
            </div>
            <div class="pl-4">
                <h3 class="text-xl font-bold"> Back-Left Leg </h3>
                <div class="flex flex-col lg:flex-row space-x-4">
                    <div class="w-full lg:w-1/4 p-2 space-y-2">
                        <p> Channel 4 (Hip Roll) </p>
                        <ValueChart :values="adcVoltages[4] ?? []" :range="[0, 3.3]" class="h-48" />
                    </div>
                    <div class="w-full lg:w-1/4 p-2 space-y-2">
                        <p> Channel 5 (Hip Pitch) </p>
                        <ValueChart :values="adcVoltages[5] ?? []" :range="[0, 3.3]" class="h-48" />
                    </div>
                    <div class="w-full lg:w-1/4 p-2 space-y-2">
                        <p> Channel 6 (Knee Pitch) </p>
                        <ValueChart :values="adcVoltages[6] ?? []" :range="[0, 3.3]" class="h-48" />
                    </div>
                    <div class="w-full lg:w-1/4 p-2 space-y-2">
                        <p> Channel 7 (Feet Contact) </p>
                        <ValueChart :values="adcVoltages[7] ?? []" :range="[0, 3.3]" class="h-48" />
                    </div>
                </div>
            </div>
            <div class="pl-4">
                <h3 class="text-xl font-bold"> Back-Right Leg </h3>
                <div class="flex flex-col lg:flex-row space-x-4">
                    <div class="w-full lg:w-1/4 p-2 space-y-2">
                        <p> Channel 8 (Hip Roll) </p>
                        <ValueChart :values="adcVoltages[8] ?? []" :range="[0, 3.3]" class="h-48" />
                    </div>
                    <div class="w-full lg:w-1/4 p-2 space-y-2">
                        <p> Channel 9 (Hip Pitch) </p>
                        <ValueChart :values="adcVoltages[9] ?? []" :range="[0, 3.3]" class="h-48" />
                    </div>
                    <div class="w-full lg:w-1/4 p-2 space-y-2">
                        <p> Channel 10 (Knee Pitch) </p>
                        <ValueChart :values="adcVoltages[10] ?? []" :range="[0, 3.3]" class="h-48" />
                    </div>
                    <div class="w-full lg:w-1/4 p-2 space-y-2">
                        <p> Channel 11 (Feet Contact) </p>
                        <ValueChart :values="adcVoltages[11] ?? []" :range="[0, 3.3]" class="h-48" />
                    </div>
                </div>
            </div>
            <div class="pl-4">
                <h3 class="text-xl font-bold"> Front-Right Leg </h3>
                <div class="flex flex-col lg:flex-row space-x-4">
                    <div class="w-full lg:w-1/4 p-2 space-y-2">
                        <p> Channel 12 (Hip Roll) </p>
                        <ValueChart :values="adcVoltages[12] ?? []" :range="[0, 3.3]" class="h-48" />
                    </div>
                    <div class="w-full lg:w-1/4 p-2 space-y-2">
                        <p> Channel 13 (Hip Pitch) </p>
                        <ValueChart :values="adcVoltages[13] ?? []" :range="[0, 3.3]" class="h-48" />
                    </div>
                    <div class="w-full lg:w-1/4 p-2 space-y-2">
                        <p> Channel 14 (Knee Pitch) </p>
                        <ValueChart :values="adcVoltages[14] ?? []" :range="[0, 3.3]" class="h-48" />
                    </div>
                    <div class="w-full lg:w-1/4 p-2 space-y-2">
                        <p> Channel 15 (Feet Contact) </p>
                        <ValueChart :values="adcVoltages[15] ?? []" :range="[0, 3.3]" class="h-48" />
                    </div>
                </div>
            </div>
        </div>
    </div>
</template>

<script lang="ts" setup>
const tny = useTNY360();

const controlLoopEnabled = ref(false);
const controlLoopLoading = ref(false);
const decisionLoopEnabled = ref(false);
const decisionLoopLoading = ref(false);

let shouldFetchInfos = false;

async function onControlLoopToggle() {
    controlLoopLoading.value = true;
    try {
        await tny.value?.system.setControlLoopEnabled(!controlLoopEnabled.value);
        controlLoopEnabled.value = !controlLoopEnabled.value;
    } catch (error) {
        console.error('Error toggling control loop:', error);
    }
    controlLoopLoading.value = false;
}

async function onDecisionLoopToggle() {
    decisionLoopLoading.value = true;
    try {
        await tny.value?.system.setDecisionLoopEnabled(!decisionLoopEnabled.value);
        decisionLoopEnabled.value = !decisionLoopEnabled.value;
    } catch (error) {
        console.error('Error toggling decision loop:', error);
    }
    decisionLoopLoading.value = false;
}

async function fetchLoopStates() {
    controlLoopLoading.value = true;
    decisionLoopLoading.value = true;
    try {
        const controlState = await tny.value?.system.getControlLoopEnabled();
        if (controlState !== undefined) {
            controlLoopEnabled.value = controlState;
        }
        controlLoopLoading.value = false;
    } catch (error) {
        console.error('Error fetching control loop state:', error);
    }
    try {
        const decisionState = await tny.value?.system.getDecisionLoopEnabled();
        if (decisionState !== undefined) {
            decisionLoopEnabled.value = decisionState;
        }
        decisionLoopLoading.value = false;
    } catch (error) {
        console.error('Error fetching decision loop state:', error);
    }
}

const adcVoltages = ref<number[][]>(Array.from({ length: 16 }, () => []));
async function fetchADCChannels() {
    console.log('fetching adc')
    try {
        const values = await tny.value?.adc.getAllChannels();
        for (let i = 0; i < 16; i++) {
            adcVoltages.value[i]?.push(values?.[i] || 0);
            if (adcVoltages.value[i]!.length > 20) {
                adcVoltages.value[i]?.shift();
            }
        }
    } catch (error) {
        if ((error as Error).message.toLowerCase().includes('timed out')) {
            // robot is overloaded, wait a bit
            await new Promise(resolve => setTimeout(resolve, 1000));
            console.warn('Timed out while fetching ADC channels, retrying...');
        } else {
            console.error('Error fetching ADC channels:', error);
        }
    }
    if (shouldFetchInfos) setTimeout(fetchADCChannels, 1);
}

onMounted(() => {
    shouldFetchInfos = true;
    fetchADCChannels();
    fetchLoopStates();
});

onUnmounted(() => {
    shouldFetchInfos = false;
});

</script>

<style></style>