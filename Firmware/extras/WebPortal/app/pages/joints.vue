<template>
    <div>
        
        <div v-for="leg in joints" :key="leg.name" class="p-4">
            <h2 class="font-bold text-xl"> {{ leg.name }} </h2>
            <div v-for="joint in leg.joints" :key="joint.index" class="p-4">
                <JointControl
                    :range="joint.range" :index="joint.index"
                    :name="joint.name" :angle="joint.angle.value"
                />
            </div>
        </div>

    </div>
</template>

<script lang="ts" setup>
const remote = useRemote();

const joints = [
    {
        name: 'Front Left Leg',
        joints: [
            { index: 7, name: 'Hip', range: [-45, 45], angle: ref(0) },
            { index: 5, name: 'Knee', range: [-135, 45], angle: ref(0) },
            { index: 6, name: 'Ankle', range: [0, 135], angle: ref(0) },
        ]
    },
    {
        name: 'Front Right Leg',
        joints: [
            { index: 4, name: 'Hip', range: [-45, 45], angle: ref(0) },
            { index: 2, name: 'Knee', range: [-135, 45], angle: ref(0) },
            { index: 3, name: 'Ankle', range: [0, 135], angle: ref(0) },
        ]
    },
    {
        name: 'Back Left Leg',
        joints: [
            { index: 10, name: 'Hip', range: [-45, 45], angle: ref(0) },
            { index: 8, name: 'Knee', range: [-135, 45], angle: ref(0) },
            { index: 9, name: 'Ankle', range: [0, 135], angle: ref(0) },
        ]
    },
    {
        name: 'Back Right Leg',
        joints: [
            { index: 13, name: 'Hip', range: [-45, 45], angle: ref(0) },
            { index: 11, name: 'Knee', range: [-135, 45], angle: ref(0) },
            { index: 12, name: 'Ankle', range: [0, 135], angle: ref(0) },
        ]
    },
];

setInterval(() => {
    remote.getAllJointAngles().then((angles) => {
        let index = 0;
        for (const leg of joints) {
            for (const joint of leg.joints) {
                joint.angle.value = angles[index] as number;
                index++;
            }
        }
    });
}, 200);

</script>

<style></style>