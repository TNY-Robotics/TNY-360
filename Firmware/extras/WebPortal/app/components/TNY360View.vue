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
let pollingInterval: number | null = null;
onMounted(() => {
    pollingInterval = setInterval(() => {
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
});

onUnmounted(() => {
    if (pollingInterval) {
        clearInterval(pollingInterval);
    }
});

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
</script>

<style></style>