<template>
    <UApp :toaster="toaster">
        <NuxtRouteAnnouncer />
        <NuxtPage />
    </UApp>
</template>

<script lang="ts" setup>
import type { ToasterProps } from '@nuxt/ui';

const toaster: ToasterProps =  { position: 'bottom-center' };
</script>