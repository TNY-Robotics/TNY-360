<template>
    <VisXYContainer :duration="0" :y-domain="[0, 5]" :x-domain="undefined" :margin="{ top: 10, bottom: 30, left: 40, right: 10 }" >
        <VisLine :data="data" :x="(d: any) => d.x" :y="(d: any) => d.y" />
        <VisAxis type="x" :gridLine="false" :domainLine="true" />
        <VisAxis type="y" :gridLine="false" :domainLine="true" />
    </VisXYContainer>
</template>

<script lang="ts" setup>
import { VisXYContainer, VisLine, VisAxis } from '@unovis/vue'
import { shallowRef, triggerRef } from 'vue'

type DataRecord = { x: number, y: number }
const data = shallowRef<DataRecord[]>([
    { x: 0, y: 0 },
    { x: 1, y: 2 },
    { x: 2, y: 1 },
    { x: 3, y: 3 },
    { x: 4, y: 2 },
    { x: 5, y: 5 },
]);
setInterval(() => {
    const lastX = data.value[data.value.length - 1]?.x || 0;
    data.value.push({ x: lastX + 1, y: Math.random() * 5 });
    if (data.value.length > 30) {
        data.value.shift();
    }
    triggerRef(data);
}, 200);
</script>

<style></style>