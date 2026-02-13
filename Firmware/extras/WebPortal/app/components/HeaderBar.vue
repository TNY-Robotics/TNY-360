<template>
    <div class="flex bg-slate-900 w-full justify-center items-center px-2 overflow-hidden">
        <div class="flex w-48 items-center">
            <h1 class="text-2xl font-black py-2 px-2 whitespace-nowrap"> TNY-360 </h1>
            <p class="text-lg px-2 italic font-light text-slate-500 whitespace-nowrap"> Dashboard </p>
        </div>
        <div class="flex grow justify-center items-center pt-1 space-x-4">
            <div v-for="page in pages" :key="page.name" class="flex flex-col w-32 user-select-none">
                <span class="flex w-full h-1 transition-all" :class="selected(page.path)? 'bg-primary-500' : 'bg-slate-800'" />
                <NuxtLink :to="page.path" class="flex justify-center items-center px-4 py-3 transition-all hover:bg-slate-800" :class="selected(page.path)? 'bg-slate-800' : 'bg-slate-900'" >
                    <p class="text-lg font-semibold">{{ page.name }}</p>
                </NuxtLink>
            </div>
        </div>
        <div class="flex justify-end items-center w-48">
            <UButton icon="i-lucide-settings" variant="link" color="neutral" size="xl" @click="openSettings" />
        </div>
    </div>
    <UModal v-model:open="settingsModalOpen" :title="'Settings'" :fullscreen="true">
        <template #body>
            Settings
        </template>
    </UModal>
</template>

<script lang="ts" setup>
const pages = [
    { name: 'Home', path: '/' },
    { name: 'Joints', path: '/joints' },
    { name: 'Movements', path: '/movements' },
    { name: 'View', path: '/view' },
];
const selected = (path: string) => {
    return path === useRoute().path;
};

const settingsModalOpen = ref(false);
function openSettings() {
    settingsModalOpen.value = true;
}
</script>

<style></style>