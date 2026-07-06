<template>
    <div class="relative p-4 w-full h-fit rounded-lg border-2 bg-white dark:bg-slate-800 border-slate-200 dark:border-slate-700">
        <div class="flex flex-wrap items-center justify-between mb-4 space-x-4">
            <div class="flex justify-center items-center space-x-4">
                <h3 :id="`error-${errorData.id}`" class="my-0"> {{ errorData?.name ?? '' }} </h3>
                <code class="font-mono bg-slate-200 dark:bg-slate-700 px-1 rounded"> {{ errorData.id }} </code>
            </div>
            <UTooltip :text="errorTypeTooltip">
                <div class="relative rounded px-2 py-1" :class="errorTypeBgColor">
                    <span class="font-semibold text-sm" :class="errorTypeTextColor">
                        {{ errorTypeText }}
                    </span>
                    <div v-if="false" class="bg-red-500/20 bg-yellow-500/20 bg-orange-500/20" />
                </div>
            </UTooltip>
        </div>

        <p class="text-slate-500 dark:text-slate-300 mb-4">{{ errorData.desc }}</p>

        <div class="flex flex-col space-y-2 bg-amber-50 dark:bg-amber-900/10 p-4 rounded-lg border border-amber-200 dark:border-amber-900/50">
            <div class="flex space-x-2 text-amber-800 dark:text-amber-500 justify-start items-center">
                <UIcon name="lucide:wrench" />
                <strong class="block">How to fix:</strong>
            </div>
            <p class="text-amber-700 dark:text-amber-400 text-sm">{{ errorData.fix }}</p>
        </div>

        <div v-if="errorPayload && errorPayload.length > 0" class="flex flex-col space-y-8 mt-4">
            <h4 class="mt-0 font-medium"> Payload Data </h4>
            <ul class="list-inside pl-4 space-y-2">
                <li v-for="item in errorPayload" :key="item.name" class="space-x-2">
                    <code class="font-mono bg-slate-200 dark:bg-slate-700 px-1 rounded">{{ item.type }}</code>
                    <span> {{ item.name }} </span>
                    <p class="inline"> = </p>
                    <code class="font-mono bg-slate-200 dark:bg-slate-700 px-1 rounded">{{ item.value }}</code>
                    <p class="inline">
                        <em class="text-slate-600 dark:text-slate-300"> {{ item.desc }} </em>
                    </p>
                </li>
            </ul>
        </div>
    </div>
</template>

<script setup lang="ts">
import typesData from '~/assets/types.json';
import errorsData from '~/assets/errors.json';
import { UInt8, UInt16, UInt32, Float32, Bool } from '@tny-robotics/sdk';

const types2Class = {
    'uint8': new UInt8(),
    'uint16': new UInt16(),
    'uint32': new UInt32(),
    'float': new Float32(),
    'bool': new Bool(),
};

const props = defineProps({
    errorCode: {
        type: String,
        required: true
    },
    payload: {
        type: Object,
        required: false,
    }
});

const errorData = computed(() => {
    return (errorsData as any)[props.errorCode] ?? {
        id: props.errorCode,
        name: 'Unknown Error',
        desc: 'No description available for this error code.',
        fix: 'No fix available for this error code.',
        payload: []
    };
});
const errorPayload = computed(() => {
    const payloadKeys = errorData.value?.payload as Array<{ name: string, type: string, desc: string, value?: any }> ?? [];

    let offset = 0;
    for (const key of payloadKeys) {
        const typeClass = (types2Class as any)[key.type];
        if (!typeClass) {
            console.warn(`Type class for ${key.type} not found. Ensure that the type is defined in types2Class.`);
            continue;
        }
        const value = typeClass.fromBytes(props.payload as ArrayBuffer, offset);
        key.value = value.value;
        if (typeClass instanceof Float32) {
            key.value = key.value.toFixed(3);
        }
        offset += value.size();
    }

    return payloadKeys;
});
const errorType = computed(() => {
    return props.errorCode.substring(props.errorCode.length - 2);
});
const errorTypeText = computed(() => {
    switch (errorType.value) {
        case '01' : return 'Trace';
        case '02' : return 'Warning';
        case '03' : return 'Error';
        case '04' : return 'Critical';
        default : return 'Unknown : ' + errorType.value;
    }
});
const errorTypeBgColor = computed(() => {
    switch (errorType.value) {
        case '02' : return 'bg-yellow-100 dark:bg-yellow-600/20';
        case '03' : return 'bg-orange-100 dark:bg-orange-600/20';
        case '04' : return 'bg-red-100 dark:bg-red-600/20';
        default : return 'bg-slate-100 dark:bg-slate-600/20';
    }
});
const errorTypeTextColor = computed(() => {
    switch (errorType.value) {
        case '02' : return 'text-yellow-800 dark:text-yellow-400';
        case '03' : return 'text-orange-800 dark:text-orange-400';
        case '04' : return 'text-red-800 dark:text-red-400';
        default : return 'text-slate-800 dark:text-slate-300';
    }
});

const errorLevelType = typesData['ErrorSeverity'] ?? null;
const errorTypeTooltip = computed(() => {
    if (!errorLevelType || !errorType.value) return '';
    return errorLevelType.values.find((item: { name: string }) => item.name.toLowerCase().trim() === errorTypeText.value.toLowerCase().trim())?.desc ?? 'No description available';
});
</script>
