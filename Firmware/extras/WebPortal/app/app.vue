<template>
    <div>
        <NuxtRouteAnnouncer />
        <div class="flex flex-col min-h-screen min-w-screen max-w-screen max-h-screen w-screen h-screen overflow-hidden">
            <HeaderBar ref="headerBar" />
            <div class="flex flex-col grow overflow-scroll" :style="{ height: `calc(100vh - ${headerBar?.offsetHeight || 0}px)` }">
                <NuxtPage />
            </div>
        </div>
    </div>
</template>

<script lang="ts" setup>
const headerBar = ref<HTMLElement | null>(null);
</script>