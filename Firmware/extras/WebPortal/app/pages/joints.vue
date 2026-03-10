<template>
    <div>
        
        <div v-for="leg in joints" :key="leg.name" class="p-4">
            <h2 class="font-bold text-xl"> {{ leg.name }} </h2>
            <div class="flex justify-evenly">
                <div v-for="joint in leg.joints" :key="joint.index" class="p-4">
                    <JointControlBox :index="joint.index" :name="joint.name" :range="joint.range" />
                </div>
            </div>
        </div>

    </div>
</template>

<script lang="ts" setup>
const joints = [
    {
        name: 'Front Left Leg',
        joints: [
            { index: 7, name: 'Hip Roll', range: [-45, 45] },
            { index: 5, name: 'Hip Pitch', range: [-135, 45] },
            { index: 6, name: 'Knee Pitch', range: [0, 135] },
        ]
    },
    {
        name: 'Front Right Leg',
        joints: [
            { index: 4, name: 'Hip Roll', range: [-45, 45] },
            { index: 2, name: 'Hip Pitch', range: [-135, 45] },
            { index: 3, name: 'Knee Pitch', range: [0, 135] },
        ]
    },
    {
        name: 'Back Left Leg',
        joints: [
            { index: 10, name: 'Hip Roll', range: [-45, 45] },
            { index: 8, name: 'Hip Pitch', range: [-135, 45] },
            { index: 9, name: 'Knee Pitch', range: [0, 135] },
        ]
    },
    {
        name: 'Back Right Leg',
        joints: [
            { index: 13, name: 'Hip Roll', range: [-45, 45] },
            { index: 11, name: 'Hip Pitch', range: [-135, 45] },
            { index: 12, name: 'Knee Pitch', range: [0, 135] },
        ]
    },
];

</script>

<style></style>