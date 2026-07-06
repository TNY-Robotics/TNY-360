<template>
    <div class="flex flex-col space-y-4">
        <div class="space-y-2">
            <h2 class="text-xl font-semibold"> Wi-Fi </h2>
            <div class="flex flex-col lg:flex-row gap-4 lg:justify-start lg:items-end lg:pl-4">
                <UFormField label="SSID">
                    <UInput v-model="ssid" type="text" />
                </UFormField>
                <UFormField label="Password">
                    <UInput v-model="password" type="password" />
                </UFormField>
                <UButton class="w-fit h-fit" label="Connect" :loading="wifiConnectBtnLoading" @click="onConnect" />
            </div>
        </div>
    </div>
</template>

<script lang="ts" setup>
const tny = useTNY360();

const ssid = ref('');
const password = ref('');

const wifiConnectBtnLoading = ref(false);
async function onConnect() {
    wifiConnectBtnLoading.value = true;
    try {
        await tny.value?.wifi.connectToAP(ssid.value, password.value);
    } catch (err) {
        console.error('Failed to connect to Wi-Fi:', err);
    }
    wifiConnectBtnLoading.value = false;
}

</script>