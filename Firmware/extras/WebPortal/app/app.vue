<template>
    <UApp :toaster="toaster">
        <NuxtRouteAnnouncer />
        <div class="flex flex-col min-h-screen min-w-screen max-w-screen max-h-screen w-screen h-screen overflow-hidden">
            <HeaderBar ref="headerBar" />
            <div class="flex flex-col w-full grow overflow-scroll" :style="{ height: `calc(100vh - ${headerBar?.offsetHeight || 0}px)` }">
                <NuxtPage />
            </div>
        </div>
    </UApp>
</template>

<script lang="ts" setup>
import type { ToasterProps } from '@nuxt/ui';

const headerBar = ref<HTMLElement | null>(null);
const toaster: ToasterProps =  { position: 'bottom-center' };
</script>