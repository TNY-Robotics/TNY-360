<template>
    <div ref="zone" class="bg-slate-900/20 rounded-full">
        <div ref="joystick" class="absolute top-0 left-0 w-20 h-20 border-2 rounded-full pointer-events-none transition-colors duration-200"
            :class="pressed? 'border-primary-500 bg-primary-500/50' : 'border-slate-500 bg-slate-500/50'">
        </div>
    </div>
</template>

<script setup lang="ts">
import { ref, onMounted } from 'vue';

const model = defineModel<{
    x: number,
    y: number
}>();

const zone = ref<HTMLElement | null>(null);
const joystick = ref<HTMLElement | null>(null);

const pressed = ref(false);
const zoneRect = ref<DOMRect | null>(null);

function onJoystickMove(x: number, y: number) {
    if (joystick.value && zoneRect.value) {
        const joystickX = (x + 1) / 2 * zoneRect.value.width - joystick.value.offsetWidth / 2;
        const joystickY = (y + 1) / 2 * zoneRect.value.height - joystick.value.offsetHeight / 2;
        joystick.value.style.transform = `translate(${joystickX}px, ${joystickY}px)`;
    }

    if (model) {
        model.value = { x, y };
    }
}

function onJoystickEvent(x: number, y: number) {
    if (!zoneRect.value) return;

    const localX = x - zoneRect.value.left;
    const localY = y - zoneRect.value.top;

    let normalizedX = (localX / zoneRect.value.width) * 2 - 1;
    let normalizedY = (localY / zoneRect.value.height) * 2 - 1;

    const vectorLength = Math.sqrt(normalizedX * normalizedX + normalizedY * normalizedY);
    if (vectorLength > 1) {
        normalizedX /= vectorLength;
        normalizedY /= vectorLength;
    }

    onJoystickMove(normalizedX, normalizedY);
}

let activeTouchId: number | null = null;

onMounted(() => {
    if (!zone.value || !joystick.value) return;

    zone.value.addEventListener('touchstart', (e) => {
        e.preventDefault();
        zoneRect.value = zone.value!.getBoundingClientRect();
        const touch = e.changedTouches[0];
        
        if (activeTouchId !== null) return;
        if (!touch) return;

        activeTouchId = touch.identifier;
        pressed.value = true;
        
        onJoystickEvent(touch.clientX, touch.clientY);
    }, { passive: false });

    zone.value.addEventListener('touchmove', (e) => {
        e.preventDefault();
        if (!pressed.value || activeTouchId === null) return;
        
        for (let i = 0; i < e.changedTouches.length; i++) {
            const touch = e.changedTouches[i];
            if (!touch) continue;

            if (touch.identifier === activeTouchId) {
                onJoystickEvent(touch.clientX, touch.clientY);
                break;
            }
        }
    }, { passive: false });

    const handleTouchEnd = (e: TouchEvent) => {
        e.preventDefault();
        if (activeTouchId === null) return;

        for (let i = 0; i < e.changedTouches.length; i++) {
            const touch = e.changedTouches[i];
            if (!touch) continue;

            if (touch.identifier === activeTouchId) {
                pressed.value = false;
                activeTouchId = null;
                onJoystickMove(0, 0);
                break;
            }
        }
    };

    zone.value.addEventListener('touchend', handleTouchEnd, { passive: false });
    zone.value.addEventListener('touchcancel', handleTouchEnd, { passive: false });

    zoneRect.value = zone.value.getBoundingClientRect();
    onJoystickMove(0, 0);
});
</script>