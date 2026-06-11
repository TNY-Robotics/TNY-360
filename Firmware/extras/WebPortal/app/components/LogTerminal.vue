<template>
    <div ref="container" class="flex flex-col space-y-2 grow bg-slate-200 dark:bg-slate-900 rounded-lg p-2 min-h-0 max-h-full overflow-y-auto">
        <div class="flex grow min-h-0 -my-0.5" />
        <LogLine v-for="(line, index) in logs" :key="index" :line="line" />
    </div>
</template>

<script lang="ts" setup>
import { useTNY360 } from '~/composables/TNY360';
import type { LogLine } from '@tny-robotics/sdk';

const tny = useTNY360();

const logs = ref<LogLine[]>([]);
let shouldPullLogs = false;
let stickToBottom = true;
let clearLogTimestamp = -1;

defineExpose({
    clear() {
        clearLogTimestamp = logs.value.length > 0 ? logs.value[logs.value.length - 1]?.timestamp_ms ?? 0 : 0;
        logs.value = [];
        stickToBottom = true;
    },
    export() {
        return logs.value.map(line => formatLogLine(line)).join('\n');
    }
});

const container = ref<HTMLDivElement>();

onMounted(() => {
    shouldPullLogs = true;
    pullLogs();

    if (container.value) {
        let lastScrollTop = container.value.scrollTop;
        container.value.addEventListener('scroll', (ev: any) => {
            if (!container.value) return;
            const scrollDirection = container.value.scrollTop - lastScrollTop;
            lastScrollTop = container.value.scrollTop;
            const distanceToBottom = container.value.scrollHeight - container.value.scrollTop - container.value.clientHeight;
            // if we are at the bottom and we scroll down, we want to stick to the bottom
            if (distanceToBottom < 10 && scrollDirection > 0) {
                stickToBottom = true;
            }
            // if we scroll up, we want to stop sticking to the bottom
            else if (scrollDirection < 0) {
                stickToBottom = false;
            }
        });
    }
});

onUnmounted(() => {
    shouldPullLogs = false;
});

function formatTimestamp(timestamp_ms: number): string {
    const minutes = Math.floor(timestamp_ms / 60000);
    const seconds = Math.floor((timestamp_ms % 60000) / 1000);
    const milliseconds = timestamp_ms % 1000;

    return `${minutes.toString().padStart(2, '0')}:${seconds.toString().padStart(2, '0')}:${milliseconds.toString().padStart(3, '0')}`;
}

function getLogLevelText(level: number): string {
    switch (level) {
        case 0: return 'INF';
        case 1: return 'WRN';
        case 2: return 'ERR';
        case 3: return 'DBG';
        case 4: return 'SUC';
        default: return '---';
    }
}

function formatLogLine(line: LogLine): string {
    const timestamp = formatTimestamp(line.timestamp_ms);
    const level = getLogLevelText(line.level);
    const tag = line.tag;
    const message = line.message;
    return `[${timestamp}] [${level}] ${' '.repeat(line.indent)}[${tag}] ${message}`;
}

async function pullLogs() {
    let logIndex = 0;
    let refreshed = false;
    let logsCount = 0;
    try {
        logsCount = await tny.value?.system.getNbLogLines() ?? 0;
    } catch (error) {}

    while (!refreshed && logIndex < logsCount) {
        // when it's all refreshed, clamp the total number of logs to 100 to avoid memory issues
        // note : even if the robot has a lower limit, we can still display more here (easier for debugging)
        const MAX_LOGS = 100;
        if (logs.value.length > MAX_LOGS) {
            logs.value.splice(0, logs.value.length - MAX_LOGS);
        }
        
        // scroll to the bottom of the container if we are close to the bottom 
        if (stickToBottom && container.value) {
            container.value.scrollTop = container.value.scrollHeight;
        }

        // get the latest log
        try {
            const logLine = await tny.value?.system.getLogLine(logIndex);
            // check if we already have this log line
            if (logLine && !logs.value.some(line => line.timestamp_ms === logLine.timestamp_ms && line.message === logLine.message)) {
                if (clearLogTimestamp !== -1 && logLine.timestamp_ms <= clearLogTimestamp) {
                    // this log line is from before the last clear, we can ignore it
                    logIndex++;
                    continue;
                }
                // we don't have this log line, add it to the logs and continue pulling
                // first find its index using timestamp
                const index = logs.value.findIndex(line => line.timestamp_ms >= logLine.timestamp_ms);
                if (index === -1) {
                    logs.value.push(logLine);
                } else {
                    logs.value.splice(index, 0, logLine);
                }
            } else {
                // we already have this log line, we can stop pulling for now
                logIndex++;
                continue;
            }
        }
        catch (error) {
            refreshed = true;
        }
        logIndex++;
    }

    // when it's all refreshed, clamp the total number of logs to 100 to avoid memory issues
    // note : even if the robot has a lower limit, we can still display more here (easier for debugging)
    const MAX_LOGS = 100;
    if (logs.value.length > MAX_LOGS) {
        logs.value.splice(0, logs.value.length - MAX_LOGS);
    }
    
    // scroll to the bottom of the container if we are close to the bottom 
    if (stickToBottom && container.value) {
        container.value.scrollTop = container.value.scrollHeight;
    }

    if (shouldPullLogs) setTimeout(pullLogs, 10);
}
</script>

<style>

</style>