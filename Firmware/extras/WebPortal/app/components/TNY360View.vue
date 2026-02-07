<template>
    <!-- Camera setup -->
    <TresPerspectiveCamera :position="[7, 7, 7]" :look-at="[0, 0, 0]" />

    <!-- Scene -->
    <TresAmbientLight :intensity="2" />
    <primitive v-if="model" :object="model.scene" />

    <!-- Visual Helpers -->
    <TresAxesHelper />
    <TresGridHelper />
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

const remote = useRemote();
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
    console.log('Registering bone:', bone.name, nameParts);
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
        console.log('Registered bone:', bone.name, 'as motor index', index, 'on axis', axis);
    } else {
        console.warn('Failed to register bone:', bone.name);
    }
}

const { state: model, isLoading, error } = useLoader(
  GLTFLoader,
  '/TNY-360.glb',
);
watchEffect(error => {
  if (error) {
    console.error('Error loading model:', error);
  }
});
watchEffect(() => {
  if (!isLoading.value) {
    console.log('Model loaded:', model);
    model.value.scene.traverse((child) => {
        if ((child as THREE.Mesh).isMesh) {
            const mesh = child as THREE.Mesh;
            mesh.castShadow = true;
            mesh.receiveShadow = true;
        }
        if ((child as THREE.Bone).isBone) {
            const bone = child as THREE.Bone;
            registerBone(bone);
        }
    } );
  }
});

const jointAngles = ref<number[]>(Array(12).fill(0));
const bodyOrientation = ref<THREE.Quaternion>(new THREE.Quaternion());
setInterval(() => {
    remote.getAllJointAngles().then((angles) => {
        jointAngles.value = angles;
    }).catch((err) => {});

    setTimeout(() => {
        remote.getBodyOrientation().then((orientation) => {
            if (model.value) {
                const quat = new THREE.Quaternion(
                    orientation.x,
                    orientation.y,
                    orientation.z,
                    orientation.w
                ).normalize();
                bodyOrientation.value = quat;
            }
        }).catch((err) => {});
    }, 50);
}, 100);

function DEG_TO_RAD(deg: number): number {
    return deg * (Math.PI / 180);
}

onBeforeRender(({ elapsed }) => {
    for (const boneStruct of bones) {
        const targetAngle = jointAngles.value[boneStruct.motorIndex] || 0;
        // Smooth interpolation
        boneStruct.angle += (targetAngle - boneStruct.angle) * 0.3;
        const rad = boneStruct.angle * (Math.PI / 180);
        const rotation = (boneStruct.invert ? -rad : rad);
        boneStruct.bone.rotation.set(
            (boneStruct.rotationAxis === 'x' ? rotation : 0) + DEG_TO_RAD(boneStruct.offsets?.x || 0),
            (boneStruct.rotationAxis === 'y' ? rotation : 0) + DEG_TO_RAD(boneStruct.offsets?.y || 0),
            (boneStruct.rotationAxis === 'z' ? rotation : 0) + DEG_TO_RAD(boneStruct.offsets?.z || 0),
        );
    }
    if (model.value) {
        model.value.scene.quaternion.set(
            bodyOrientation.value.x,
            bodyOrientation.value.y,
            bodyOrientation.value.z,
            bodyOrientation.value.w
        );
    }
});

// setInterval(() => {
//     jointAngles.value[0] = 0;
//     jointAngles.value[1] = 0;
//     jointAngles.value[2] = 0;
//     jointAngles.value[3] = 0;
//     jointAngles.value[4] = 0;
//     jointAngles.value[5] = 0;
//     jointAngles.value[6] = 0;
//     jointAngles.value[7] = 0;
//     jointAngles.value[8] = 0;
//     jointAngles.value[9] = 0;
//     jointAngles.value[10] = 0;
//     jointAngles.value[11] = 0;
//     setTimeout(() => {
//         jointAngles.value[0] = 45;
//         jointAngles.value[3] = 45;
//         jointAngles.value[6] = 45;
//         jointAngles.value[9] = 45;
//     }, 500);
//     setTimeout(() => {
//         jointAngles.value[1] = 45;
//         jointAngles.value[4] = 45;
//         jointAngles.value[7] = 45;
//         jointAngles.value[10] = 45;
//     }, 1000);
//     setTimeout(() => {
//         jointAngles.value[2] = 45;
//         jointAngles.value[5] = 45;
//         jointAngles.value[8] = 45;
//         jointAngles.value[11] = 45;
//     }, 1500);
// }, 3000);
</script>

<style></style>