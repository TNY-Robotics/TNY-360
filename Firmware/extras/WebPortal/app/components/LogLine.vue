<template>
    <div class="w-full text-sm font-mono">
        <span class="text-slate-700 dark:text-slate-300 font-mono">
            [{{ formatTimestamp(props.line.timestamp_ms) }}]
        </span>
        <span :class="`text-${getLogLevelColor(props.line.level)}-500 font-mono`">
            [{{ getLogLevelText(props.line.level) }}]
        </span>
        <span :class="`text-${getLogLevelColor(props.line.level)}-800 dark:text-${getLogLevelColor(props.line.level)}-200 font-mono`" :style="`padding-left: ${props.line.indent*10}px`">
            [{{ props.line.tag }}]
        </span>
        <span :class="`text-${getLogLevelColor(props.line.level)}-800 dark:text-${getLogLevelColor(props.line.level)}-200 font-mono`">
            {{ props.line.message }}
        </span>

        <span v-if="false" class="text-red-500 text-yellow-500 text-green-500 text-blue-500 text-gray-500" />
        <span v-if="false" class="text-red-800 text-yellow-800 text-green-800 text-blue-800 text-gray-800" />
        <span v-if="false" class="dark:text-red-200 dark:text-yellow-200 dark:text-green-200 dark:text-blue-200 dark:text-gray-200" />
    </div>
</template>

<script lang="ts" setup>
import type { LogLine } from '@tny-robotics/sdk';
import { LogLevel } from '@tny-robotics/sdk';

const props = defineProps<{
    line: LogLine;
}>();

function getLogLevelColor(level: LogLevel): string {
    switch (level) {
        case LogLevel.Info: return 'blue';
        case LogLevel.Warning: return 'yellow';
        case LogLevel.Error: return 'red';
        case LogLevel.Debug: return 'gray';
        case LogLevel.Success: return 'green';
        default: return 'gray';
    }
}

function getLogLevelText(level: LogLevel): string {
    switch (level) {
        case LogLevel.Info: return 'INF';
        case LogLevel.Warning: return 'WRN';
        case LogLevel.Error: return 'ERR';
        case LogLevel.Debug: return 'DBG';
        case LogLevel.Success: return 'SUC';
        default: return '---';
    }
}

function formatTimestamp(timestamp_ms: number): string {
    const minutes = Math.floor(timestamp_ms / 60000);
    const seconds = Math.floor((timestamp_ms % 60000) / 1000);
    const milliseconds = timestamp_ms % 1000;

    return `${minutes.toString().padStart(2, '0')}:${seconds.toString().padStart(2, '0')}:${milliseconds.toString().padStart(3, '0')}`;
}
</script>

<style>
.font-mono {
    font-family: monospace !important;
}
</style>