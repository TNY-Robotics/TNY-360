<template>
    <div class="flex bg-slate-200 dark:bg-slate-900 w-full justify-center items-center px-2">
        <div class="relative flex w-full lg:w-48 justify-between items-center">
            <div class="flex justify-start items-center w-16 lg:w-fit">
                <UButton icon="lucide:arrow-left" size="xl" variant="ghost" color="neutral" @click="goBack" />
            </div>
            <h1 class="text-2xl font-black py-2 px-2 whitespace-nowrap"> TNY-360 </h1>
            <p class="text-lg px-2 italic font-light text-slate-600 dark:text-slate-500 whitespace-nowrap"> Dashboard </p>
            <div class="flex lg:hidden justify-end items-center w-16 lg:w-fit">
                <UButton icon="lucide:menu" size="xl" variant="ghost" color="neutral" @click="toggleMobileMenu" />
            </div>
            <div ref="mobileMenu" class="absolute top-full left-0 w-full min-h-0 bg-white dark:bg-slate-900 shadow-lg rounded-b-lg overflow-hidden transition-all duration-300 ease-in-out z-50">
                <div class="w-full h-fit min-h-fit gap-4 p-4">
                    <div v-for="page in pages" :key="page.name" class="flex flex-col w-full user-select-none">
                        <NuxtLink :to="page.path" class="flex justify-center items-center px-4 py-2 transition-all rounded-lg hover:bg-slate-300 hover:dark:bg-slate-800" :class="selected(page.path)? 'bg-slate-50 dark:bg-slate-800' : 'bg-slate-100 dark:bg-slate-900'" >
                            <p class="text-lg font-semibold" :class="selected(page.path) ? 'text-primary-500': ''">{{ page.name }}</p>
                        </NuxtLink>
                    </div>
                    <div class="flex w-full justify-end">
                        <UButton icon="lucide:settings" variant="link" color="neutral" size="xl" @click="openSettings" />
                    </div>
                </div>
            </div>
        </div>
        <div class="hidden lg:flex grow justify-center items-center pt-1 space-x-4">
            <div v-for="page in pages" :key="page.name" class="flex flex-col w-32 user-select-none">
                <span class="flex w-full h-1 transition-all" :class="selected(page.path)? 'bg-primary-500 dark:bg-primary-500' : 'bg-slate-400 dark:bg-slate-800'" />
                <NuxtLink :to="page.path" class="flex justify-center items-center px-4 py-3 transition-all hover:bg-slate-300 hover:dark:bg-slate-800" :class="selected(page.path)? 'bg-slate-50 dark:bg-slate-800' : 'bg-slate-100 dark:bg-slate-900'" >
                    <p class="text-lg font-semibold">{{ page.name }}</p>
                </NuxtLink>
            </div>
        </div>
        <div class="hidden lg:flex justify-end items-center w-48">
            <UButton icon="lucide:settings" variant="link" color="neutral" size="xl" @click="openSettings" />
        </div>
    </div>
    <UModal v-model:open="settingsModalOpen" :title="'Settings'" :fullscreen="true">
        <template #body>
            <SettingsModal />
        </template>
    </UModal>
</template>

<script lang="ts" setup>
const router = useRouter();

const pages = [
    { name: 'Home', path: '/dashboard/' },
    { name: 'Joints', path: '/dashboard/joints' },
    { name: 'Movements', path: '/dashboard/movements' },
    { name: 'View', path: '/dashboard/view' },
    { name: 'Advanced', path: '/dashboard/advanced' },
];
const selected = (path: string) => {
    return path === useRoute().path || path === useRoute().path + '/';
};

const settingsModalOpen = ref(false);
function openSettings() {
    settingsModalOpen.value = true;
}

function goBack() {
    router.replace('/');
}

const mobileMenu = ref<HTMLElement | null>(null);
const mobileMenuOpen = ref(false);
function toggleMobileMenu() {
    mobileMenuOpen.value = !mobileMenuOpen.value;
}
watch(mobileMenuOpen, (newValue) => {
    onMobileMenuChange(newValue);
});

function onMobileMenuChange(open: boolean) {
    if (!mobileMenu.value) return;
    if (open) {
        const child = mobileMenu.value.firstElementChild as HTMLElement;
        const rect = child.getBoundingClientRect();
        mobileMenu.value.style.height = rect.height + 'px';
    } else {
        mobileMenu.value.style.height = '0px';
    }
}

onMounted(() => {
    onMobileMenuChange(mobileMenuOpen.value);
});

watch(useRoute(), () => {
    mobileMenuOpen.value = false;
});
</script>

<style></style>