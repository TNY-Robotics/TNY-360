<template>
    <div class="relative min-h-screen min-w-screen max-w-screen max-h-screen w-screen h-screen overflow-hidden">
        <!-- CAMERA FEED -->
        <div class="absolute top-0 left-0 w-full h-full flex items-center justify-center bg-black dark:bg-black">
            <img
                class="w-full h-full object-cover" 
                :src="streamUrl ?? ''"
            />
        </div>

        <!-- BACK BUTTON -->
        <UButton class="absolute top-4 left-4" variant="ghost" color="neutral" @click="goBack" icon="lucide:arrow-left" />

        <!-- TOP STATUS BAR -->
        <div class="absolute top-2 right-0 left-0 h-fit flex justify-center items-center pointer-events-none">
            <div class="w-fit h-fit bg-slate-200/30 dark:bg-slate-800/70 border border-white/50 dark:border-slate-800 rounded-lg shadow-lg px-2 py-1">
                <p class="text-sm text-slate-900 dark:text-slate-50"> Latency: <code class="bg-white/50 dark:bg-slate-950/50 px-1 rounded-sm">{{ Math.round(tny?.latency ?? 0) }} ms</code> </p>
            </div>
        </div>

        <!-- LEFT JOYSTICK : MOVE X, Y -->
        <Joystick class="absolute bottom-0 left-0 w-64 h-64" v-model="joyLeft" />
        <!-- RIGHT JOYSTICK : ROTATE Z + LOOK UP/DOWN -->
        <Joystick class="absolute bottom-0 right-0 w-64 h-64" v-model="joyRight" />
    </div>
</template>

<script lang="ts" setup>
const router = useRouter();

const tny = useTNY360();
const tnyAddr = ref<string | null>(null);

const streamUrl = computed(() => {
    return tny.value?.ip ? `${window.location.protocol}//${tny.value.ip}:90?t=${Date.now()}` : null;
});

watch(tny, (newVal) => {
    if (!tny.value) return;
    tnyAddr.value = tny.value.ip;
});

const joyLeft = ref({
    x: 0,
    y: 0
});
const joyRight = ref({
    x: 0,
    y: 0
});

function goBack() {
    if (document.fullscreenElement) {
        document.exitFullscreen();
    }
    if (screen.orientation && screen.orientation.unlock) {
        screen.orientation.unlock();
    }
    router.replace('/');
}

let shouldSendCommands = false;
async function sendCommandsContinuous() {
    try {
        await tny.value?.body.setVelocity(-joyLeft.value.y*0.45, -joyLeft.value.x*0.45, -joyRight.value.x*2.8, false);
    } catch (err) { await new Promise((resolve) => setTimeout(resolve, 1000)); }
    if (shouldSendCommands) setTimeout(sendCommandsContinuous, 50);
}

async function enableRobot() {
    try {
        const enabledFlag = await tny.value?.body.getEnabled();
        if (enabledFlag === undefined) throw new Error('Failed to get enabled flag');
        if (((enabledFlag as number) & 0b111111111111) <= 0)
        {
            const time = await tny.value?.body.enableSmooth();
            if (!time) throw new Error('Failed to run smooth enable');
            await new Promise((resolve) => setTimeout(resolve, time * 1000));
        }
    } catch (err) {
        console.error(err);
    }
}

async function disableRobot() {
    try {
        const enabledFlag = await tny.value?.body.getEnabled();
        if (enabledFlag === undefined) throw new Error('Failed to get enabled flag');
        if (((enabledFlag as number) & 0b111111111111) > 0)
        {
            const time = await tny.value?.body.disableSmooth();
            if (!time) throw new Error('Failed to run smooth disable');
            await new Promise((resolve) => setTimeout(resolve, time * 1000));
        }
    } catch (err) {
        console.error(err);
    }
}

onMounted(() => {
    // request fullscreen on page load
    if (document.fullscreenEnabled) {
        document.documentElement.requestFullscreen().catch((err) => {
            console.error(`Error attempting to enable full-screen mode: ${err.message} (${err.name})`);
        });
    }
    // also fix rotation on mobile devices
    if (screen.orientation && screen.orientation.lock) {
        screen.orientation.lock('landscape').catch((err) => {
            console.error(`Error attempting to lock screen orientation: ${err.message} (${err.name})`);
        });
    }

    // listen when the user exits fullscreen, this means he wants to go back to the home page
    document.addEventListener('fullscreenchange', () => {
        if (!document.fullscreenElement) {
            router.replace('/');
        }
    });

    enableRobot();

    shouldSendCommands = true;
    sendCommandsContinuous();
});

onUnmounted(() => {
    shouldSendCommands = false;

    disableRobot();
});
</script>

<style>
.backdrop-blur {
    backdrop-filter: blur(16px);
}
</style>