<template>
    <div class="flex flex-col min-h-screen min-w-screen max-w-screen max-h-screen w-screen h-screen overflow-hidden p-4 lg:p-8">
        <p class="text-center show-up text-6xl font-extrabold"> TNY - 360 </p>
        <p class="text-center show-up text-lg text-gray-500 dark:text-gray-500"> Bienvenue sur le portail web de votre TNY-360 !</p>
        <div class="flex grow justify-center items-center">
            <div class="flex flex-col space-y-8 h-fit w-fit justify-center items-center">
                <UButton class="show-up delay-200 flex justify-between w-fit" size="xl" variant="subtle" href="/teleop" label="Jouer avec mon TNY-360" icon="lucide:gamepad-2" />
                <UButton class="show-up delay-400 flex justify-between w-fit" size="xl" variant="subtle" href="/dashboard" label="Programmer mon TNY-360" icon="lucide:chart-no-axes-gantt" />
                <UButton class="show-up delay-600 flex justify-between w-fit" size="xl" variant="subtle" href="/dashboard" label="Voir le Panneau de Contrôle" icon="lucide:gauge" />
            </div>
        </div>
    </div>
</template>

<script lang="ts" setup>

</script>

<style></style>