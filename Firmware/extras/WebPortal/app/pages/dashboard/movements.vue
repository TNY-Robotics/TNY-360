<template>
    <div class="relative">
        <div class="flex flex-col space-y-4 p-4">
            <h2>Body control</h2>
            <div class="flex flex-col space-y-2 p-2">
                <p class="text-lg">Translation X</p>
                <div class="flex space-x-4">
                    <USlider v-model="body.pos_x" :min="-10" :max="10" :step="0.5" class="w-lg" />
                    <code class="w-12 bg-slate-200 dark:bg-slate-900 text-center px-2 py-0.5 rounded-md font-light">{{ body.pos_x }}</code>
                    <p>cm</p>
                </div>
            </div>
            <div class="flex flex-col space-y-2 p-2">
                <p class="text-lg">Translation Y</p>
                <div class="flex space-x-4">
                    <USlider v-model="body.pos_y" :min="-10" :max="10" :step="0.5" class="w-lg" />
                    <code class="w-12 bg-slate-200 dark:bg-slate-900 text-center px-2 py-0.5 rounded-md font-light">{{ body.pos_y }}</code>
                    <p>cm</p>
                </div>
            </div>
            <div class="flex flex-col space-y-2 p-2">
                <p class="text-lg">Translation Z</p>
                <div class="flex space-x-4">
                    <USlider v-model="body.pos_z" :min="5" :max="20" :step="0.5" class="w-lg" />
                    <code class="w-12 bg-slate-200 dark:bg-slate-900 text-center px-2 py-0.5 rounded-md font-light">{{ body.pos_z }}</code>
                    <p>cm</p>
                </div>
            </div>
            <div class="flex flex-col space-y-2 p-2">
                <p class="text-lg">Rotation X</p>
                <div class="flex space-x-4">
                    <USlider v-model="body.rot_x" :min="-45" :max="45" :step="0.1" class="w-lg" />
                    <code class="w-12 bg-slate-200 dark:bg-slate-900 text-center px-2 py-0.5 rounded-md font-light">{{ body.rot_x }}</code>
                    <p>deg</p>
                </div>
            </div>
            <div class="flex flex-col space-y-2 p-2">
                <p class="text-lg">Rotation Y</p>
                <div class="flex space-x-4">
                    <USlider v-model="body.rot_y" :min="-45" :max="45" :step="0.1" class="w-lg" />
                    <code class="w-12 bg-slate-200 dark:bg-slate-900 text-center px-2 py-0.5 rounded-md font-light">{{ body.rot_y }}</code>
                    <p>deg</p>
                </div>
            </div>
            <div class="flex flex-col space-y-2 p-2">
                <p class="text-lg">Rotation Z</p>
                <div class="flex space-x-4">
                    <USlider v-model="body.rot_z" :min="-45" :max="45" :step="0.1" class="w-lg" />
                    <code class="w-12 bg-slate-200 dark:bg-slate-900 text-center px-2 py-0.5 rounded-md font-light">{{ body.rot_z }}</code>
                    <p>deg</p>
                </div>
            </div>
        </div>
        <div class="flex flex-col space-y-4 p-4">
            <h2>Movement Control</h2>
            <div class="flex flex-col space-y-2 p-2">
                <p class="text-lg">Translation speed</p>
                <div class="flex space-x-4">
                    <USlider v-model="translation_speed" :min="0.0" :max="0.8" :step="0.05" class="w-lg" />
                    <code class="w-12 bg-slate-200 dark:bg-slate-900 text-center px-2 py-0.5 rounded-md font-light">{{ translation_speed }}</code>
                    <p class="whitespace-nowrap">m/s</p>
                </div>
            </div>
            <div class="flex flex-col space-y-2 p-2">
                <p class="text-lg">Rotation speed</p>
                <div class="flex space-x-4">
                    <USlider v-model="rotation_speed" :min="0" :max="180" :step="5" class="w-lg" />
                    <code class="w-12 bg-slate-200 dark:bg-slate-900 text-center px-2 py-0.5 rounded-md font-light">{{ rotation_speed }}</code>
                    <p class="whitespace-nowrap">deg/s</p>
                </div>
            </div>
        </div>
        <div class="absolute top-2 right-2">
            <p>Select Gamepad</p>
            <USelect v-model="selectedGamepad" :items="gamepadItems" class="w-48 lg:w-64" />
        </div>
    </div>
</template>

<script lang="ts" setup>
import { DEG2RAD } from 'three/src/math/MathUtils';


const tny = useTNY360();

const DEFAULT_HEIGHT = 12.5; // cm

// body control
const body = reactive({
    pos_x: 1.5, // cm
    pos_y: 0,
    pos_z: DEFAULT_HEIGHT,
    rot_x: 0, // deg
    rot_y: 0,
    rot_z: 0,
});

watch(body, () => {
    tny.value?.body.setPosture(
        body.pos_x / 100, // cm to m
        body.pos_y / 100, // cm to m
        body.pos_z / 100, // cm to m
        body.rot_x * DEG2RAD, // degrees to radians
        body.rot_y * DEG2RAD, // degrees to radians
        body.rot_z * DEG2RAD, // degrees to radians
        true
    );
}, { deep: true });

// movement control

const keys = {
    px: false,
    mx: false,
    py: false,
    my: false,
    pz: false,
    mz: false,
};
const translation_speed = ref(0.25); // m/s
const rotation_speed = ref(80); // deg/s

onMounted(() => {
    window.addEventListener('keydown', (event) => {
        switch (event.code) {
            case 'KeyW':
                keys.px = true;
                break;
            case 'KeyS':
                keys.mx = true;
                break;
            case 'KeyA':
                keys.py = true;
                break;
            case 'KeyD':
                keys.my = true;
                break;
            case 'ArrowLeft':
                keys.pz = true;
                break;
            case 'ArrowRight':
                keys.mz = true;
                break;
        }
        updateMovement();
    });

    window.addEventListener('keyup', (event) => {
        switch (event.code) {
            case 'KeyW':
                keys.px = false;
                break;
            case 'KeyS':
                keys.mx = false;
                break;
            case 'KeyA':
                keys.py = false;
                break;
            case 'KeyD':
                keys.my = false;
                break;
            case 'ArrowLeft':
                keys.pz = false;
                break;
            case 'ArrowRight':
                keys.mz = false;
                break;
        }
        updateMovement();
    });

    gamepadList.value = [];
    for (const gpd of navigator.getGamepads()) {
        if (gpd !== null) gamepadList.value.push(gpd);
    }
});

onUnmounted(() => {
    shouldPoll = false;
})

const movement = reactive({
    x: 0,
    y: 0,
    z: 0,
});

async function updateMovement() {
    movement.x = (keys.px ? translation_speed.value : 0) - (keys.mx ? translation_speed.value : 0);
    movement.y = (keys.py ? translation_speed.value : 0) - (keys.my ? translation_speed.value : 0);
    movement.z = (keys.pz ? rotation_speed.value : 0) - (keys.mz ? rotation_speed.value : 0);
    await tny.value?.body.setVelocity(movement.x, movement.y, movement.z * DEG2RAD, true); // deg/s to rad/s
}

const gamepadList = ref<Gamepad[]>([]);
const gamepadItems = computed<any[]>(() => {
    const items = [{ label: 'None', value: null as any }];
    gamepadList.value.map(gp => ({
        label: gp.id,
        value: gp.id,
    })).forEach(item => items.push(item));
    return items;
});

window.addEventListener('gamepadconnected', (event) => {
    console.debug('gamepag connected');
    gamepadList.value.push(event.gamepad);
});
window.addEventListener('gamepaddisconnected', (event) => {
    console.debug('gamepad disconnected');
    gamepadList.value = gamepadList.value.filter(gp => gp.index !== event.gamepad.index);
});

const selectedGamepad = ref<any>(null);
watch(selectedGamepad, (newGamepad: any) => {
    console.log(`Selected gamepad: ${newGamepad}`);
    stopGamepadPolling();
    if (newGamepad) {
        const gp = navigator.getGamepads().find(g => g?.id === newGamepad);
        if (gp) {
            startGamepadPolling(gp);
        }
    }
});

let shouldPoll = true;
async function pollGamepad(gamepad: Gamepad) {
    const gp = navigator.getGamepads()[gamepad.index];
    if (gp) {
        const leftStickX = gp.axes[0] || 0;
        const leftStickY = gp.axes[1] || 0;
        const rightStickX = gp.axes[2] || 0;
        const deadzone = 0.1;

        const x = Math.abs(leftStickX) > deadzone ? leftStickX : 0;
        const y = Math.abs(leftStickY) > deadzone ? leftStickY : 0;
        const z = Math.abs(rightStickX) > deadzone ? rightStickX : 0;

        movement.x = -y * translation_speed.value;
        movement.y = -x * translation_speed.value;
        movement.z = -z * rotation_speed.value;

        try { await tny.value?.body.setVelocity(movement.x, movement.y, movement.z * DEG2RAD, true); }
        catch (err) {}

        const dpadUp = gp.buttons[12]?.pressed;
        const dpadDown = gp.buttons[13]?.pressed;
        const dpadLeft = gp.buttons[14]?.pressed;
        const dpadRight = gp.buttons[15]?.pressed;

        const bodyAngle = 15; // degrees
        const bodyAngleY = ((dpadUp ? 1 : 0) + (dpadDown ? -1 : 0)) * bodyAngle;
        const bodyAngleX = ((dpadLeft ? -1 : 0) + (dpadRight ? 1 : 0)) * bodyAngle;
        try {
            await tny.value?.body.setPosture(
                body.pos_x / 100,
                body.pos_y / 100,
                body.pos_z / 100,
                bodyAngleX * DEG2RAD,
                bodyAngleY * DEG2RAD,
                0,
            );
        } catch (err) {}
    }

    if (shouldPoll) setTimeout(() => pollGamepad(gamepad), 100);
}

function stopGamepadPolling() {
    shouldPoll = false;
}
function startGamepadPolling(gamepad: Gamepad) {
    shouldPoll = true;
    pollGamepad(gamepad);
}

</script>

<style></style>