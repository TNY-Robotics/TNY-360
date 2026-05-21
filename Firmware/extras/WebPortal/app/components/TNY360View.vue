<template>
    <!-- Camera setup -->
    <TresPerspectiveCamera :position="[7, 7, 7]" :look-at="[0, 0, 0]" :up="[0, 0, 1]" />

    <!-- Scene -->
    <TresAmbientLight :intensity="2" />
    <primitive v-if="model" :object="model.scene" />

    <!-- Visual Helpers -->
    <TresAxesHelper />
    <TresGridHelper :rotation="[-Math.PI / 2, 0, 0]" :args="[10, 10]" />
    <TresOrbitControls
        v-if="camera"
        :args="[camera, renderer?.domElement]"
    />
</template>

<script lang="ts" setup>
import { useLoop, useTres, useLoader, extend } from '@tresjs/core';
import { GLTFLoader } from 'three/examples/jsm/loaders/GLTFLoader.js';
import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls.js';
import * as THREE from 'three';
import { RAD2DEG } from 'three/src/math/MathUtils';

const toast = useToast();

const tny = useTNY360();
const { camera, renderer } = useTres();
const { onBeforeRender } = useLoop();

extend({ OrbitControls });

type BoneStruct = {
    motorIndex: number;
    rotationAxis: 'x' | 'y' | 'z';
    bone: THREE.Bone;
    angle: number;
    offsets?: {
        x: number;
        y: number;
        z: number;
    }
    invert?: boolean;
};
const bones: BoneStruct[] = [];

function registerBone(bone: THREE.Bone) {
    const nameParts = bone.name.split('_');
    if (nameParts.length < 3) {
        console.warn('Bone name does not conform to expected format:', bone.name);
        return;
    }
    const index = parseInt(nameParts[1] as string);
    const axisPart = nameParts[2] as string;
    const invertRotation = axisPart.startsWith("-");
    const axis = axisPart.replace("-", "").toLocaleLowerCase() as 'x' | 'y' | 'z';
    if (!isNaN(index) && (axis === 'x' || axis === 'y' || axis === 'z')) {
        bones.push({
            motorIndex: index,
            rotationAxis: axis,
            bone: bone,
            angle: 0,
            offsets: {
                x: parseFloat(nameParts[3] || '0'),
                y: parseFloat(nameParts[4] || '0'),
                z: parseFloat(nameParts[5] || '0'),
            },
            invert: invertRotation,
        });
    } else {
        console.warn('Failed to register bone:', bone.name);
    }
}

const modelLoadingToast = toast.add({
    title: 'Loading 3D Model',
    description: 'Loading the TNY-360 model, please wait...',
    duration: 0, // Make it persistent until manually dismissed
});
const { state: model, isLoading, error } = useLoader(GLTFLoader, '/TNY-360.glb');
watchEffect(() => {
  if (!isLoading.value && model.value) {
    model.value.scene.traverse((child) => {
        if ((child as THREE.Mesh).isMesh) {
            const mesh = child as THREE.Mesh;
            mesh.castShadow = false;
            mesh.receiveShadow = false;
        }
        if ((child as THREE.Bone).isBone) {
            const bone = child as THREE.Bone;
            registerBone(bone);
        }
    });
    setTimeout(() => {
        toast.update(modelLoadingToast.id, {
            title: 'Model Loaded',
            description: 'The TNY-360 model has been loaded.',
            color: 'success',
            duration: 2000,
        });
    }, 10);
  }

  if (error.value) {
    setTimeout(() => {
        toast.update(modelLoadingToast.id, {
            title: 'Error Loading Model',
            description: 'There was an error loading the 3D model.',
            color: 'error',
            duration: 4000,
        });
    }, 10);
    console.error('Error loading model:', error.value);
  }
});

let shouldFetch = false;

const lastJointAngles = Array(12).fill(0);
const currentJointAngles = Array(12).fill(0);
const lastOrientation = { x: 0, y: 0, z: 0 };
const currentOrientation = { x: 0, y: 0, z: 0 };

async function fetchAngleContinuous() {
    try {
        const angles = await tny.value?.joint.getJointAngles();
        if (angles) {
            for (let i = 0; i < angles.length; i++) {
                lastJointAngles[i] = (angles[i] ?? 0) * RAD2DEG;
            }
        }
    } catch (err) { console.warn('Could not get joint angles', err); }

    try {
        const orientation = await tny.value?.imu.getOrientation() ?? { x_rad: 0, y_rad: 0, z_rad: 0 };
        if (model.value) {
            lastOrientation.x = orientation.x_rad;
            lastOrientation.y = orientation.y_rad;
            lastOrientation.z = orientation.z_rad;
        }
    } catch (err) { console.warn('Could not get imu orientation', err); }

    if (shouldFetch) setTimeout(fetchAngleContinuous, 1);
} 

onMounted(() => {
    setTimeout(() => {
        shouldFetch = true;
        fetchAngleContinuous();
    }, 10);
});

onUnmounted(() => {
    shouldFetch = false;
});

function DEG_TO_RAD(deg: number): number {
    return deg * (Math.PI / 180);
}

onBeforeRender(({ elapsed }) => {

    // Update orientation and joints
    const alpha = 0.2;
    currentOrientation.x += (lastOrientation.x - currentOrientation.x) * alpha;
    currentOrientation.y += (lastOrientation.y - currentOrientation.y) * alpha;
    currentOrientation.z += (lastOrientation.z - currentOrientation.z) * alpha;
    for (let i = 0; i < lastJointAngles.length; i++) {
        currentJointAngles[i] += (lastJointAngles[i] - currentJointAngles[i]) * alpha;
        const boneStruct = bones.find(b => b.motorIndex === i);
        if (boneStruct) {
            boneStruct.angle = currentJointAngles[i];
        }
    }

    // apply to model
    if (model.value) {
        model.value.scene.quaternion.setFromEuler(
            new THREE.Euler(
                currentOrientation.x,
                currentOrientation.y,
                currentOrientation.z,
                'XYZ'
            )
        );
        for (const boneStruct of bones) {
            const rad = boneStruct.angle * (Math.PI / 180);
            const rotation = (boneStruct.invert ? -rad : rad);
            boneStruct.bone.rotation.set(
                (boneStruct.rotationAxis === 'x' ? rotation : 0) + DEG_TO_RAD(boneStruct.offsets?.x || 0),
                (boneStruct.rotationAxis === 'y' ? rotation : 0) + DEG_TO_RAD(boneStruct.offsets?.y || 0),
                (boneStruct.rotationAxis === 'z' ? rotation : 0) + DEG_TO_RAD(boneStruct.offsets?.z || 0),
            );
        }
    }
});
</script>

<style></style>