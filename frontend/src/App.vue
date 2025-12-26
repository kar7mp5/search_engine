<template>
    <div
        class="max-h-screen flex flex-col items-center justify-center bg-white p-4"
    >
        <!-- Logo / Title -->
        <h1 class="text-5xl font-bold mb-10">Search Engine</h1>

        <!-- Form -->
        <form @submit.prevent="handleSearch" class="w-full max-w-xl">
            <input
                v-model="query"
                type="text"
                placeholder="Enter starting URL (e.g. https://example.com)"
                autofocus
                required
                class="w-full px-5 py-4 text-xl border border-black focus:outline-none mb-6"
            />

            <div class="flex justify-center gap-6">
                <button
                    type="submit"
                    :disabled="loading"
                    class="px-8 py-3 text-base font-medium border border-black bg-white hover:bg-gray-100 disabled:opacity-50 disabled:cursor-not-allowed"
                >
                    {{ loading ? "Crawling..." : "Search" }}
                </button>

                <button
                    type="button"
                    :disabled="loading"
                    class="px-8 py-3 text-base font-medium border border-black bg-white hover:bg-gray-100 disabled:opacity-50 disabled:cursor-not-allowed"
                >
                    I'm Feeling Lucky
                </button>
            </div>

            <!-- Status / Error message -->
            <div
                v-if="message"
                class="mt-8 text-center text-lg"
                :class="error ? 'text-red-600' : 'text-green-600'"
            >
                {{ message }}
            </div>

            <!-- Results -->
            <div v-if="results.length > 0" class="mt-10 w-full">
                <h2 class="text-2xl font-semibold mb-4 text-center">
                    Discovered Base URLs
                </h2>
                <ul
                    class="list-disc pl-8 space-y-2 text-left max-w-2xl mx-auto"
                >
                    <li
                        v-for="(url, index) in results"
                        :key="index"
                        class="text-lg break-all"
                    >
                        {{ url }}
                    </li>
                </ul>
            </div>
        </form>

        <!-- Footer links -->
        <div class="mt-16 text-sm flex gap-8">
            <a href="#" class="hover:underline">About</a>
            <a href="#" class="hover:underline">Privacy</a>
            <a href="#" class="hover:underline">Terms</a>
        </div>
    </div>
</template>

<script setup>
import { ref } from "vue";
import { crawlWebsites } from "@/services/api";

const query = ref("");
const loading = ref(false);
const message = ref("");
const error = ref(false);
const results = ref([]);

const handleSearch = async () => {
    if (!query.value.trim()) return;

    loading.value = true;
    message.value = "";
    error.value = false;
    results.value = [];

    const payload = {
        start_urls: [query.value.trim()],
        max_depth: 2,
        num_threads: 4,
    };

    const result = await crawlWebsites(payload);

    loading.value = false;

    console.log("API result:", result);

    if (result.success) {
        message.value = result.data?.message || "Crawl completed!";
        results.value = result.data?.base_urls || [];
        if (results.value.length === 0) {
            message.value += " (no URLs found)";
        }
        query.value = ""; // clear input
    } else {
        message.value = result.error || "Something went wrong";
        error.value = true;
    }
};
</script>
