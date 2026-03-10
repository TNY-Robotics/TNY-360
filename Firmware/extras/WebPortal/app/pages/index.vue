<template>
    <div class="p-4">
        <div class="flex space-x-4">
            <div class="p-2">
                <h2 class="text-xl font-semibold"> CPU Usage ({{ cpu_temp.toFixed(0) }}°C) </h2>
                <div class="flex space-x-8 p-2">
                    <div>
                        <p>Core 0</p>
                        <BarChart class="w-32"
                            :nbars="10" direction="vertical" :size="2" v-model="cpuUsage.core0"
                        />
                        <p class="text-center w-full"> {{Math.round(cpuUsage.core0 * 100)}} % </p>
                    </div>
                    <div>
                        <p>Core 1</p>
                        <BarChart class="w-32"
                            :nbars="10" direction="vertical" :size="2" v-model="cpuUsage.core1"
                        />
                        <p class="text-center w-full"> {{Math.round(cpuUsage.core1 * 100)}} % </p>
                    </div>
                </div>
            </div>
            <div class="p-2">
                <h2 class="text-xl font-semibold"> RAM Usage </h2>
                <div class="flex space-x-8 p-2">
                    <div>
                        <p>Internal</p>
                        <BarChart class="w-32"
                            :nbars="10" direction="vertical" :size="2" v-model="ramUsage.internal_percent"
                        />
                        <p class="text-center w-full"> {{formatRAM(ramUsage.internal_used)}} / {{ formatRAM(ramUsage.internal_total) }} </p>
                    </div>
                    <div>
                        <p>PSRAM</p>
                        <BarChart class="w-32"
                            :nbars="10" direction="vertical" :size="2" v-model="ramUsage.psram_percent"
                        />
                        <p class="text-center w-full"> {{formatRAM(ramUsage.psram_used)}} / {{ formatRAM(ramUsage.psram_total) }} </p>
                    </div>
                </div>
            </div>
        </div>
    </div>
</template>

<script lang="ts" setup>
const remote = useRemote();

const cpu_temp = ref(0);
const cpuUsage = ref<{core0: number, core1: number}>({core0: 0, core1: 0});
const ramUsage = ref<{internal_total: number, internal_used: number, psram_total: number, psram_used: number, internal_percent: number, psram_percent: number}>({
    internal_total: 0,
    internal_used: 0,
    internal_percent: 0,
    psram_total: 0,
    psram_used: 0,
    psram_percent: 0
});

let pollingInterval : number | null = null;
onMounted(() => {
    pollingInterval = setInterval(async  () => {
        const cpu_usage = await remote.getCPUUsage();
        cpuUsage.value.core0 = cpu_usage.core0 / 100;
        cpuUsage.value.core1 = cpu_usage.core1 / 100;
            
        const ram_usage = await remote.getRAMUsage();
        ramUsage.value.internal_total = ram_usage.internal_total;
        ramUsage.value.internal_used = ram_usage.internal_used;
        ramUsage.value.psram_total = ram_usage.psram_total;
        ramUsage.value.psram_used = ram_usage.psram_used;
        ramUsage.value.internal_percent = ram_usage.internal_used / ram_usage.internal_total;
        ramUsage.value.psram_percent = ram_usage.psram_used / ram_usage.psram_total

        const temp = await remote.getCPUTemp();
        cpu_temp.value = temp;
    }, 1000);
});

onUnmounted(() => {
    if (pollingInterval) {
        clearInterval(pollingInterval);
    }
});

function formatRAM(bytes: number): string {
    if (bytes < 1024) return bytes + ' B';
    else if (bytes < 1024 * 1024) return (bytes / 1024).toFixed(0) + ' KB';
    else if (bytes < 1024 * 1024 * 1024) return (bytes / (1024 * 1024)).toFixed(0) + ' MB';
    else return (bytes / (1024 * 1024 * 1024)).toFixed(0) + ' GB';
}

</script>

<style></style>