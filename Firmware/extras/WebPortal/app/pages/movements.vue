<template>
    <div class="relative">
        <div class="flex flex-col space-y-4 p-4">
            <h1 class="text-2xl font-bold">Body control</h1>
            <div class="flex flex-col space-y-2 p-2">
                <p class="text-xl">Translation X</p>
                <div class="flex space-x-4">
                    <USlider v-model="body.pos_x" :min="-20" :max="20" :step="1" class="w-lg" />
                    <code class="w-12 bg-slate-900 text-center px-2 py-0.5 rounded-md font-light">{{ body.pos_x }}</code>
                    <p>cm</p>
                </div>
            </div>
            <div class="flex flex-col space-y-2 p-2">
                <p class="text-xl">Translation Y</p>
                <div class="flex space-x-4">
                    <USlider v-model="body.pos_y" :min="-20" :max="20" :step="1" class="w-lg" />
                    <code class="w-12 bg-slate-900 text-center px-2 py-0.5 rounded-md font-light">{{ body.pos_y }}</code>
                    <p>cm</p>
                </div>
            </div>
            <div class="flex flex-col space-y-2 p-2">
                <p class="text-xl">Translation Z</p>
                <div class="flex space-x-4">
                    <USlider v-model="body.pos_z" :min="0" :max="20" :step="1" class="w-lg" />
                    <code class="w-12 bg-slate-900 text-center px-2 py-0.5 rounded-md font-light">{{ body.pos_z }}</code>
                    <p>cm</p>
                </div>
            </div>
            <div class="flex flex-col space-y-2 p-2">
                <p class="text-xl">Rotation X</p>
                <div class="flex space-x-4">
                    <USlider v-model="body.rot_x" :min="-45" :max="45" :step="1" class="w-lg" />
                    <code class="w-12 bg-slate-900 text-center px-2 py-0.5 rounded-md font-light">{{ body.rot_x }}</code>
                    <p>deg</p>
                </div>
            </div>
            <div class="flex flex-col space-y-2 p-2">
                <p class="text-xl">Rotation Y</p>
                <div class="flex space-x-4">
                    <USlider v-model="body.rot_y" :min="-45" :max="45" :step="1" class="w-lg" />
                    <code class="w-12 bg-slate-900 text-center px-2 py-0.5 rounded-md font-light">{{ body.rot_y }}</code>
                    <p>deg</p>
                </div>
            </div>
            <div class="flex flex-col space-y-2 p-2">
                <p class="text-xl">Rotation Z</p>
                <div class="flex space-x-4">
                    <USlider v-model="body.rot_z" :min="-45" :max="45" :step="1" class="w-lg" />
                    <code class="w-12 bg-slate-900 text-center px-2 py-0.5 rounded-md font-light">{{ body.rot_z }}</code>
                    <p>deg</p>
                </div>
            </div>
        </div>
        <div class="flex flex-col space-y-4 p-4">
            <h1 class="text-2xl font-bold">Movement Control</h1>
            <div class="flex flex-col space-y-2 p-2">
                <p class="text-xl">Translation speed</p>
                <div class="flex space-x-4">
                    <USlider v-model="translation_speed" :min="0.0" :max="1" :step="0.01" class="w-lg" />
                    <code class="w-12 bg-slate-900 text-center px-2 py-0.5 rounded-md font-light">{{ translation_speed }}</code>
                    <p>m/s</p>
                </div>
            </div>
            <div class="flex flex-col space-y-2 p-2">
                <p class="text-xl">Rotation speed</p>
                <div class="flex space-x-4">
                    <USlider v-model="rotation_speed" :min="0.0" :max="180" :step="1" class="w-lg" />
                    <code class="w-12 bg-slate-900 text-center px-2 py-0.5 rounded-md font-light">{{ rotation_speed }}</code>
                    <p>deg/s</p>
                </div>
            </div>
        </div>
        <div class="absolute top-2 right-2">
            <p>Select Gamepad</p>
            <USelect v-model="selectedGamepad" :items="gamepadItems" class="w-64" />
        </div>
    </div>
</template>

<script lang="ts" setup>

const remote = useRemote();

// body control
const body = reactive({
    pos_x: 2,
    pos_y: 0,
    pos_z: 12,
    rot_x: 0,
    rot_y: 0,
    rot_z: 0,
});

watch(body, () => {
    remote.setBodyPosture(
        body.rot_x,
        body.rot_y,
        body.rot_z,
        body.pos_x,
        body.pos_y,
        body.pos_z
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
const translation_speed = ref(0.40);
const rotation_speed = ref(80);

onMounted(() => {
    remote.setBodyPosture(
        body.rot_x,
        body.rot_y,
        body.rot_z,
        body.pos_x,
        body.pos_y,
        body.pos_z
    ); // send at the start the initial posture

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
});

const movement = reactive({
    x: 0,
    y: 0,
    z: 0,
});

function updateMovement() {
    movement.x = (keys.px ? translation_speed.value : 0) - (keys.mx ? translation_speed.value : 0);
    movement.y = (keys.py ? translation_speed.value : 0) - (keys.my ? translation_speed.value : 0);
    movement.z = (keys.pz ? rotation_speed.value : 0) - (keys.mz ? rotation_speed.value : 0);
    remote.setMovementVelocity(movement.x, movement.y, movement.z);
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
    gamepadList.value.push(event.gamepad);
});
window.addEventListener('gamepaddisconnected', (event) => {
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

let intervalId: number | null = null;
function stopGamepadPolling() {
    if (intervalId !== null) {
        clearInterval(intervalId);
        intervalId = null;
    }
}
function startGamepadPolling(gamepad: Gamepad) {
    intervalId = setInterval(() => {
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

            remote.setMovementVelocity(movement.x, movement.y, movement.z);
            console.log(`Gamepad movement: x=${movement.x.toFixed(2)} m/s, y=${movement.y.toFixed(2)} m/s, z=${movement.z.toFixed(2)} deg/s`);

            setTimeout(() => {
                const dpadUp = gp.buttons[12]?.pressed;
                const dpadDown = gp.buttons[13]?.pressed;
                const dpadLeft = gp.buttons[14]?.pressed;
                const dpadRight = gp.buttons[15]?.pressed;

                const bodyAngle = 15; // degrees
                const bodyAngleY = ((dpadUp ? 1 : 0) + (dpadDown ? -1 : 0)) * bodyAngle;
                const bodyAngleX = ((dpadLeft ? -1 : 0) + (dpadRight ? 1 : 0)) * bodyAngle;
                remote.setBodyPosture(
                    bodyAngleX,
                    bodyAngleY,
                    0,
                    body.pos_x,
                    body.pos_y,
                    body.pos_z
                );
                // console.log(`Gamepad body posture: rot_x=${bodyAngleX} deg, rot_y=${bodyAngleY} deg`);
            }, 100);
        }
    }, 200); // Polling period in ms
}

</script>

<style></style>