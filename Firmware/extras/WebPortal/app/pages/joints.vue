<template>
    <div>
        
        <div v-for="leg in joints" :key="leg.name" class="p-4">
            <div class="flex items-center justify-start space-x-4">
                <h2 class="font-bold text-xl"> {{ leg.name }} </h2>
                <UButton :color="leg.enableLoading? 'neutral' : (leg.enabled? 'success': 'error')" variant="soft" @click="toggleLegEnabled(leg)" :icon="leg.enabled? 'i-lucide-check' : 'i-lucide-x'" trailing :loading="leg.enableLoading">
                    {{ leg.enabled? 'Enabled' : 'Disabled' }}
                </UButton>
            </div>
            <div class="flex justify-evenly">
                <div v-for="joint in leg.joints" :key="joint.index" class="p-4">
                    <JointControlBox :index="joint.index" :name="joint.name" :range="joint.range" />
                </div>
            </div>
        </div>

    </div>
</template>

<script lang="ts" setup>
import { LegJointFlag } from '@tny-robotics/sdk';

const robot = useTNY360();

const joints = reactive([
    {
        index: 0,
        name: 'Front Left Leg',
        joints: [
            { index: 0, name: 'Hip Roll', range: [-45, 45] },
            { index: 1, name: 'Hip Pitch', range: [-135, 45] },
            { index: 2, name: 'Knee Pitch', range: [0, 135] },
        ],
        enabled: false,
        enableLoading: false
    },
    {
        index: 1,
        name: 'Back Left Leg',
        joints: [
            { index: 3, name: 'Hip Roll', range: [-45, 45] },
            { index: 4, name: 'Hip Pitch', range: [-135, 45] },
            { index: 5, name: 'Knee Pitch', range: [0, 135] },
        ],
        enabled: false,
        enableLoading: false
    },
    {
        index: 2,
        name: 'Back Right Leg',
        joints: [
            { index: 6, name: 'Hip Roll', range: [-45, 45] },
            { index: 7, name: 'Hip Pitch', range: [-135, 45] },
            { index: 8, name: 'Knee Pitch', range: [0, 135] },
        ],
        enabled: false,
        enableLoading: false
    },
    {
        index: 3,
        name: 'Front Right Leg',
        joints: [
            { index: 9, name: 'Hip Roll', range: [-45, 45] },
            { index: 10, name: 'Hip Pitch', range: [-135, 45] },
            { index: 11, name: 'Knee Pitch', range: [0, 135] },
        ],
        enabled: false,
        enableLoading: false
    },
]);

async function toggleLegEnabled(leg: any) {
    leg.enableLoading = true;
    leg.enabled = !leg.enabled;
    await robot.value?.leg.setEnabled(leg.index, leg.enabled ? LegJointFlag.HipPitch | LegJointFlag.KneePitch | LegJointFlag.HipRoll : 0, true);
    leg.enableLoading = false;
}

</script>

<style></style>