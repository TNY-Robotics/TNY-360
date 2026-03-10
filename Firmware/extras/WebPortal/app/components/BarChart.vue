<template>
    <div class="flex" :class="direction === 'vertical' ? 'flex-col' : 'flex-row'">
        <span v-for="i in nbars" :key="i"
            class="rounded m-0.5"
            :class="[
                (direction === 'vertical' ? (model || 0) >= 1-(i/nbars) : (model || 0) <= 1-(i/nbars)) ? 'bg-primary-500' : 'bg-gray-300 dark:bg-gray-700',
                direction === 'vertical' ? 'w-full' : `h-full`,
                direction === 'vertical' ? `h-${size}` : `w-${size}`
            ]"
        />
    </div>
</template>

<script lang="ts" setup>
const props = defineProps<{
    nbars: number;
    direction: 'vertical' | 'horizontal';
    size: number;
}>();
const model = defineModel<number>();
</script>
