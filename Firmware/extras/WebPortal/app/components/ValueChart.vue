<template>
    <VisXYContainer :duration="0" :y-domain="props.range || [0, 5]" :x-domain="undefined" >
        <VisLine :data="data" :x="(d: any) => d.x" :y="(d: any) => d.y" />
        <VisAxis type="x" :gridLine="false" :domainLine="true" />
        <VisAxis type="y" :gridLine="false" :domainLine="true" />
    </VisXYContainer>
</template>

<script lang="ts" setup>
import { VisXYContainer, VisLine, VisAxis } from '@unovis/vue'

const props = defineProps<{
    values: number[];
    range?: [number, number];
}>();
const data = computed(() => {
    return props.values.map((value, index) => ({ x: index, y: value }));
});
</script>

<style></style>