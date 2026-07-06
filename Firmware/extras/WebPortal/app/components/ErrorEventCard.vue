<template>
    <div class="flex flex-col space-y-8">
        <div>
            <ApiError :errorCode="formatErrorCode(errorEvent)" :payload="errorEvent?.payload" v-if="errorEvent" />
        </div>
        <div class="flex flex-col w-full">
            <h3> Linked Events : </h3>
            <div class="flex flex-col gap-4 p-4">
                <div v-if="linkedEvents === undefined" class="flex justify-center items-center">
                    <UIcon name="lucide:loader-2" class="animate-spin w-8 h-8 text-gray-500 dark:text-gray-400" />
                </div>
                <ApiError class="show-up" v-for="event in linkedEvents" :key="event.eventId" :errorCode="formatErrorCode(event)" :payload="event.payload" />
            </div>
        </div>
    </div>
</template>

<script lang="ts" setup>
import type { ErrorEvent } from '@tny-robotics/sdk';
const robot = useTNY360();

const props = defineProps<{
    code: number | undefined;
    id: number | undefined;
}>();

watch(props, onInfoUpdate, { immediate: true, deep: true });

function toHex(val: number, len: number = 2): string {
    return val.toString(16).toUpperCase().padStart(len, '0');
}

const errorEvent = ref<ErrorEvent | null>(null);
function formatErrorCode(ev: ErrorEvent) {
    return `0x${toHex(ev.module)}${toHex(ev.subsystem)}${toHex(ev.code)}${toHex(ev.severity)}`;
}

async function onInfoUpdate() {
    if (props.id !== undefined) {
        try {
            const event = await robot.value?.error.getErrorEventById(props.id);
            if (event) {
                errorEvent.value = event;
                console.log('Fetched error event:', event);
            } else {
                console.warn(`No error event found for ID: ${props.id}`);
                errorEvent.value = null;
            }
        } catch (error) {
            console.error('Failed to fetch error event:', error);
        }

        try {
            searchErrorTrace();
        } catch (err) {
            console.error('Failed to search error trace:', err);
        }
    }
}

const linkedEvents = ref<ErrorEvent[]|undefined>(undefined);
async function searchErrorTrace() {
    linkedEvents.value = undefined; // mark loading state

    const count = await robot.value?.error.getErrorCount();
    if (count === undefined) return console.warn('Error count is undefined');
    
    const errorTrace: ErrorEvent[] = [];
    for (let i = 0; i < count; i++) {
        const event = await robot.value?.error.getErrorEventByIndex(i);
        if (!event) return console.warn(`No error event found at index: ${i}`);
        errorTrace.push(event);
    }
    if (errorTrace.length === 0) return console.warn('Error trace is empty');

    const foundErrorInTrace = errorTrace.find(e => e.eventId === props.id);
    if (!foundErrorInTrace) {
        console.warn(`Error event with ID ${props.id} not found in error trace`);
        return;
    }
    
    const errorTimestamp = foundErrorInTrace.timestampMs;
    linkedEvents.value = errorTrace.filter(e => Math.abs(e.timestampMs - errorTimestamp) < 2 && e.eventId !== props.id);
}
</script>

<style></style>