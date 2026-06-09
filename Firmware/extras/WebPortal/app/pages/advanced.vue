<template>
    <div class="p-4 space-y-4">
        <h2>ADC Voltages</h2>
        <div class="space-y-8">
            <div class="pl-4">
                <h3 class="text-xl font-bold"> Front-Left Leg </h3>
                <div class="flex space-x-4">
                    <div class="w-1/4 p-2 space-y-2">
                        <p> Channel 1 (Hip Roll) </p>
                        <ValueChart :values="adcVoltages[0] ?? []" :range="[0, 3.3]" class="h-48" />
                    </div>
                    <div class="w-1/4 p-2 space-y-2">
                        <p> Channel 2 (Hip Pitch) </p>
                        <ValueChart :values="adcVoltages[1] ?? []" :range="[0, 3.3]" class="h-48" />
                    </div>
                    <div class="w-1/4 p-2 space-y-2">
                        <p> Channel 3 (Knee Pitch) </p>
                        <ValueChart :values="adcVoltages[2] ?? []" :range="[0, 3.3]" class="h-48" />
                    </div>
                    <div class="w-1/4 p-2 space-y-2">
                        <p> Channel 4 (Feet Contact) </p>
                        <ValueChart :values="adcVoltages[3] ?? []" :range="[0, 3.3]" class="h-48" />
                    </div>
                </div>
            </div>
            <div class="pl-4">
                <h3 class="text-xl font-bold"> Back-Left Leg </h3>
                <div class="flex space-x-4">
                    <div class="w-1/4 p-2 space-y-2">
                        <p> Channel 4 (Hip Roll) </p>
                        <ValueChart :values="adcVoltages[4] ?? []" :range="[0, 3.3]" class="h-48" />
                    </div>
                    <div class="w-1/4 p-2 space-y-2">
                        <p> Channel 5 (Hip Pitch) </p>
                        <ValueChart :values="adcVoltages[5] ?? []" :range="[0, 3.3]" class="h-48" />
                    </div>
                    <div class="w-1/4 p-2 space-y-2">
                        <p> Channel 6 (Knee Pitch) </p>
                        <ValueChart :values="adcVoltages[6] ?? []" :range="[0, 3.3]" class="h-48" />
                    </div>
                    <div class="w-1/4 p-2 space-y-2">
                        <p> Channel 7 (Feet Contact) </p>
                        <ValueChart :values="adcVoltages[7] ?? []" :range="[0, 3.3]" class="h-48" />
                    </div>
                </div>
            </div>
            <div class="pl-4">
                <h3 class="text-xl font-bold"> Back-Right Leg </h3>
                <div class="flex space-x-4">
                    <div class="w-1/4 p-2 space-y-2">
                        <p> Channel 8 (Hip Roll) </p>
                        <ValueChart :values="adcVoltages[8] ?? []" :range="[0, 3.3]" class="h-48" />
                    </div>
                    <div class="w-1/4 p-2 space-y-2">
                        <p> Channel 9 (Hip Pitch) </p>
                        <ValueChart :values="adcVoltages[9] ?? []" :range="[0, 3.3]" class="h-48" />
                    </div>
                    <div class="w-1/4 p-2 space-y-2">
                        <p> Channel 10 (Knee Pitch) </p>
                        <ValueChart :values="adcVoltages[10] ?? []" :range="[0, 3.3]" class="h-48" />
                    </div>
                    <div class="w-1/4 p-2 space-y-2">
                        <p> Channel 11 (Feet Contact) </p>
                        <ValueChart :values="adcVoltages[11] ?? []" :range="[0, 3.3]" class="h-48" />
                    </div>
                </div>
            </div>
            <div class="pl-4">
                <h3 class="text-xl font-bold"> Front-Right Leg </h3>
                <div class="flex space-x-4">
                    <div class="w-1/4 p-2 space-y-2">
                        <p> Channel 12 (Hip Roll) </p>
                        <ValueChart :values="adcVoltages[12] ?? []" :range="[0, 3.3]" class="h-48" />
                    </div>
                    <div class="w-1/4 p-2 space-y-2">
                        <p> Channel 13 (Hip Pitch) </p>
                        <ValueChart :values="adcVoltages[13] ?? []" :range="[0, 3.3]" class="h-48" />
                    </div>
                    <div class="w-1/4 p-2 space-y-2">
                        <p> Channel 14 (Knee Pitch) </p>
                        <ValueChart :values="adcVoltages[14] ?? []" :range="[0, 3.3]" class="h-48" />
                    </div>
                    <div class="w-1/4 p-2 space-y-2">
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

const adcVoltages = ref<number[][]>(Array.from({ length: 16 }, () => []));
async function fetchInfos() {
    try {
        const values = await tny.value?.adc.getAllChannels();
        for (let i = 0; i < 16; i++) {
            adcVoltages.value[i]?.push(values?.[i] || 0);
            if (adcVoltages.value[i]!.length > 20) {
                adcVoltages.value[i]?.shift();
            }
        }
    } catch (error) { console.error('Error fetching ADC voltages:', error); }
}

let fetchInterval: number | null = null;
onMounted(() => {
    fetchInterval = setInterval(fetchInfos, 100);
});

onUnmounted(() => {
    if (fetchInterval !== null) {
        clearInterval(fetchInterval);
    }
});

</script>

<style></style>