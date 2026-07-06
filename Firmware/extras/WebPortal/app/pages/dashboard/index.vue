<template>
    <div class="flex flex-col lg:flex-row grow p-2 lg:p-4 min-h-0 max-h-full gap-4">
        <div class="show-right flex flex-col w-full lg:w-1/2 lg:space-y-4">
            <div class="flex flex-col lg:flex-row gap-2 lg:gap-4">
                <div class="px-2">
                    <h2> CPU Usage ({{ Math.round(stats?.temp_c * 10) / 10 }}°C) </h2>
                    <div class="flex space-x-8 p-2">
                        <div>
                            <p>Core 0</p>
                            <BarChart class="w-32"
                                :nbars="10" direction="vertical" :size="2" v-model="stats.cpu_usage.core0_percent.value"
                            />
                            <p class="text-center w-full"> {{Math.round(stats?.cpu_usage.core0 * 100)}} % </p>
                        </div>
                        <div>
                            <p>Core 1</p>
                            <BarChart class="w-32"
                                :nbars="10" direction="vertical" :size="2" v-model="stats.cpu_usage.core1_percent.value"
                            />
                            <p class="text-center w-full"> {{Math.round(stats?.cpu_usage.core1 * 100)}} % </p>
                        </div>
                    </div>
                </div>
                <div class="px-2">
                    <h2> RAM Usage </h2>
                    <div class="flex space-x-8 p-2">
                        <div>
                            <p>Internal</p>
                            <BarChart class="w-32"
                                :nbars="10" direction="vertical" :size="2" v-model="stats.ram_usage.internal_percent.value"
                            />
                            <p class="text-center w-full"> {{formatRAM(stats?.ram_usage.internal_used)}} / {{ formatRAM(stats?.ram_usage.internal_total) }} </p>
                        </div>
                        <div>
                            <p>PSRAM</p>
                            <BarChart class="w-32"
                                :nbars="10" direction="vertical" :size="2" v-model="stats.ram_usage.psram_percent.value"
                            />
                            <p class="text-center w-full"> {{formatRAM(stats?.ram_usage.psram_used)}} / {{ formatRAM(stats?.ram_usage.psram_total) }} </p>
                        </div>
                    </div>
                </div>
            </div>
            <div class="flex space-x-4 w-full">
                <h2> Latency :</h2>
                <p class="text-lg lg:text-2xl font-mono"> {{ latency !== null ? Math.round(latency) + ' ms' : 'N/A' }} </p>
            </div>
        </div>
        <div class="show-left flex h-full flex-col w-full lg:w-1/2 space-y-4 max-h-full min-h-0">
            <div class="flex w-full justify-between">
                <h2> Logs </h2>
                <div class="flex space-x-4">
                    <UButton icon="lucide:square-arrow-right-exit" trailing label="Export" variant="subtle" color="neutral" @click="exportLogs" />
                    <UButton label="Clear" variant="solid" color="error" @click="clearLogs" />
                </div>
            </div>
            <LogTerminal ref="logTerminal" class="min-h-32" />
        </div>
    </div>
</template>

<script lang="ts" setup>
import { useTNY360 } from '~/composables/TNY360';

const tny = useTNY360();

const stats = {
    temp_c: 0,
    cpu_usage: {
        core0: 0,
        core0_percent: ref(0),
        core1: 0,
        core1_percent: ref(0),
    },
    ram_usage: {
        internal_total: 0,
        internal_used: 0,
        internal_percent: ref(0),
        psram_total: 0,
        psram_used: 0,
        psram_percent: ref(0),
    }
}

let shouldPullStats = false;
let shouldUpdateLatency = false;
const latency = ref(null as number | null);
onMounted(() => {
    shouldPullStats = true;
    shouldUpdateLatency = true;
    pullStatisticsContinuous();
    updateLatencyContinuous();
});

onUnmounted(() => {
    shouldPullStats = false;
    shouldUpdateLatency = false;
});

async function updateLatencyContinuous() {
    latency.value = tny.value?.latency ?? null;
    if (shouldUpdateLatency) setTimeout(updateLatencyContinuous, 500);
}

async function pullStatisticsContinuous() {
    try {
        const res = await tny.value?.system.getStatistics();
        if (res) {
            stats.temp_c = res.temp_c;
            stats.cpu_usage.core0 = res.cpu_usage.core0;
            stats.cpu_usage.core1 = res.cpu_usage.core1;
            stats.ram_usage.internal_used = res.ram_usage.internal_used;
            stats.ram_usage.internal_total = res.ram_usage.internal_total;
            stats.ram_usage.psram_used = res.ram_usage.psram_used;
            stats.ram_usage.psram_total = res.ram_usage.psram_total;

            stats.cpu_usage.core0_percent.value = res.cpu_usage.core0;
            stats.cpu_usage.core1_percent.value = res.cpu_usage.core1;
            stats.ram_usage.internal_percent.value = res.ram_usage.internal_used / res.ram_usage.internal_total;
            stats.ram_usage.psram_percent.value = res.ram_usage.psram_used / res.ram_usage.psram_total;
        }
    } catch (error) {
        if ((error as Error).message.toLowerCase().includes('timed out')) {
            // robot is overloaded, wait a bit
            await new Promise(resolve => setTimeout(resolve, 1000));
            console.warn('Timed out while fetching statistics, retrying...');
        }
        else console.error('Error fetching statistics:', error);
    }
    if (shouldPullStats) setTimeout(pullStatisticsContinuous, 500);
}

function formatRAM(bytes: number): string {
    if (bytes < 1024) return bytes + ' B';
    else if (bytes < 1024 * 1024) return (bytes / 1024).toFixed(0) + ' KB';
    else if (bytes < 1024 * 1024 * 1024) return (bytes / (1024 * 1024)).toFixed(0) + ' MB';
    else return (bytes / (1024 * 1024 * 1024)).toFixed(0) + ' GB';
}

const logTerminal = ref<InstanceType<typeof import('~/components/LogTerminal.vue').default>>();
function exportLogs() {
    if (logTerminal.value) {
        const logs = logTerminal.value.export();
        const blob = new Blob([logs], { type: 'text/plain' });
        const url = URL.createObjectURL(blob);
        const a = document.createElement('a');
        a.href = url;
        a.download = 'logs.txt';
        document.body.appendChild(a);
        a.click();
        document.body.removeChild(a);
        URL.revokeObjectURL(url);
    }
}

function clearLogs() {
    if (logTerminal.value) {
        logTerminal.value.clear();
    }
}

</script>

<style></style>