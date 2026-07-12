<template>
    <div>
        <div v-for="(leg, index) in joints" :key="leg.name" class="show-up p-2 lg:p-4" :class="`delay-${index * 100}`">
            <div class="flex items-center justify-start space-x-4">
                <h2 class="font-bold text-xl"> {{ leg.name }} </h2>
            </div>
            <div class="flex flex-col lg:flex-row justify-evenly">
                <div v-for="joint in leg.joints" :key="joint.driverIndex" class="p-4">
                    <JointControlBox :driverIndex="joint.driverIndex" :readerIndex="joint.readerIndex" :name="joint.name" :range="joint.range" :shouldUpdate="shouldUpdate" v-model:blockUpdates="joint.blockUpdates" />
                </div>
            </div>
        </div>
    </div>
</template>

<script lang="ts" setup>
import { LegJointFlag } from '@tny-robotics/sdk';

const robot = useTNY360();

const shouldUpdate = ref(true);
const joints = reactive([
    {
        index: 0,
        name: 'Front Left Leg',
        joints: [
            { driverIndex: 0, readerIndex: 0, name: 'Hip Roll', range: [-45, 45], blockUpdates: false },
            { driverIndex: 1, readerIndex: 1, name: 'Hip Pitch', range: [-135, 45], blockUpdates: false },
            { driverIndex: 2, readerIndex: 2, name: 'Knee Pitch', range: [0, 155], blockUpdates: false },
        ],
    },
    {
        index: 1,
        name: 'Back Left Leg',
        joints: [
            { driverIndex: 3, readerIndex: 4, name: 'Hip Roll', range: [-45, 45], blockUpdates: false },
            { driverIndex: 4, readerIndex: 5, name: 'Hip Pitch', range: [-135, 45], blockUpdates: false },
            { driverIndex: 5, readerIndex: 6, name: 'Knee Pitch', range: [0, 155], blockUpdates: false },
        ],
    },
    {
        index: 2,
        name: 'Back Right Leg',
        joints: [
            { driverIndex: 6, readerIndex: 8, name: 'Hip Roll', range: [-45, 45], blockUpdates: false },
            { driverIndex: 7, readerIndex: 9, name: 'Hip Pitch', range: [-135, 45], blockUpdates: false },
            { driverIndex: 8, readerIndex: 10, name: 'Knee Pitch', range: [0, 155], blockUpdates: false },
        ],
    },
    {
        index: 3,
        name: 'Front Right Leg',
        joints: [
            { driverIndex: 9, readerIndex: 12, name: 'Hip Roll', range: [-45, 45], blockUpdates: false },
            { driverIndex: 10, readerIndex: 13, name: 'Hip Pitch', range: [-135, 45], blockUpdates: false },
            { driverIndex: 11, readerIndex: 14, name: 'Knee Pitch', range: [0, 155], blockUpdates: false },
        ],
    },
]);

watch(joints, () => {
    // for all joints, if one asks to block, we block everything
    shouldUpdate.value = !joints.some(leg => leg.joints.some(joint => joint.blockUpdates));
}, { deep: true });

</script>

<style></style>