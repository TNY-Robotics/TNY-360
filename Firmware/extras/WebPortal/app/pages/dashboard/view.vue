<template>
    <TresCanvas :clear-alpha="0" :antialias="true">
        <TNY360View class="show" />
    </TresCanvas>
</template>

<script lang="ts" setup>
import { TresCanvas } from '@tresjs/core';
</script>

<style></style>