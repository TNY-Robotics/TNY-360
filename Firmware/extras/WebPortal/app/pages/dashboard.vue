<template>
    <div class="flex flex-col min-h-screen min-w-screen max-w-screen max-h-screen w-screen h-screen overflow-hidden">
        <HeaderBar ref="headerBar" class="show-down" />
        <div class="flex flex-col w-full grow overflow-scroll" :style="{ height: `calc(100vh - ${headerBar?.offsetHeight || 0}px)` }">
            <NuxtPage />
        </div>
    </div>
</template>

<script setup lang="ts">
const headerBar = ref<HTMLElement | null>(null);

</script>