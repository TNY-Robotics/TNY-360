<template>
    <UModal :title="`Advanced - ${name}`" v-model:open="modelOpen">
        <template #body>
            <div class="flex flex-col space-y-8 w-full h-full overflow-hidden pb-2">
                <div class="flex flex-col space-y-4">
                    <div class="flex justify-between items-center w-full">
                        <h3>Duty Cycle</h3>
                        <UButton class="w-fit" :color="motorEnabled? 'success' : 'error'" variant="soft" @click="motorEnabled = !motorEnabled" :icon="motorEnabled? 'i-lucide-check' : 'i-lucide-x'" trailing>
                            {{ motorEnabled? 'Enabled' : 'Disabled' }}
                        </UButton>
                    </div>
                    <div class="flex space-x-4">
                        <UInput v-model="sliderMin" class="flex w-16" />
                        <div class="flex w-full relative">
                            <div class="absolute w-full h-0 top-6 left-0" :style="{ transform: `translateX(${((sliderValue - sliderMinNbr) / (sliderMaxNbr - sliderMinNbr)) * 96 + 2}%)` }">
                                <p :style="{ transform: 'translateX(-50%)' }" class="w-fit">{{ sliderValue.toFixed(2) }}</p>
                            </div>
                            <USlider :min="sliderMinNbr" :max="sliderMaxNbr" :step="0.01" v-model="sliderValue" />
                        </div>
                        <UInput v-model="sliderMax" class="flex w-16" />
                    </div>
                </div>
                <hr class="border-slate-200 dark:border-slate-700" />
                <div class="flex flex-col space-y-4">
                    <div class="flex justify-between items-center w-full">
                        <h3>Feedback</h3>
                        <div class="w-24 bg-slate-100 dark:bg-slate-700 rounded px-2 py-1 text-center">
                            {{ adcVoltages[adcVoltages.length - 1]?.toFixed(2) ?? 'N/A' }} V
                        </div>
                    </div>
                    <ValueChart :values="adcVoltages ?? []" :range="[0, 3.3]" class="h-48" />
                </div>
            </div>
        </template>
    </UModal>
</template>

<script setup lang="ts">
const tny = useTNY360();

const props = defineProps<{
    driverIndex: number,
    readerIndex: number,
    name: string
}>();

const modelOpen = defineModel<boolean>('open');

const sliderMin = ref("0.5");
const sliderMax = ref("2.5");
const sliderValue = ref(1.5);

const sliderMinNbr = computed(() => parseFloat(sliderMin.value));
const sliderMaxNbr = computed(() => parseFloat(sliderMax.value));

const motorEnabled = ref(false);
const adcVoltages = ref<number[]>([]);

let shouldUpdate = false;
watch(modelOpen, (newVal) => {
    shouldUpdate = newVal ?? false;
    if (newVal) updateLoop();
});

async function updateLoop() {
    if (!modelOpen.value) return;

    try {
        await tny.value?.motor.setDutyCycle(props.driverIndex, motorEnabled.value? sliderValue.value : 0, true);
        const voltages = await tny.value?.adc.getAllChannels();
        if (voltages?.[props.readerIndex]) adcVoltages.value.push(voltages?.[props.readerIndex] ?? 0);
    } catch (e) {
        console.error('Failed to update motor or ADC values', e);
        await new Promise(resolve => setTimeout(resolve, 500)); // let some time to recover
    }

    if (shouldUpdate) setTimeout(updateLoop, 50);
}

</script>
