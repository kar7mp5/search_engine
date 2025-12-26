<template>
    <div
        class="min-h-screen bg-gradient-to-br from-blue-50 to-purple-100 flex items-center justify-center p-8"
    >
        <div
            class="max-w-lg w-full bg-white rounded-3xl shadow-2xl p-10 transform transition hover:scale-105"
        >
            <h1 class="text-5xl font-extrabold text-gray-800 mb-6 text-center">
                Vue 3 + Tailwind + Yarn
            </h1>
            <p class="text-xl text-gray-600 text-center mb-8">It works! 🚀</p>
            <div class="flex justify-center gap-6">
                <button
                    class="px-8 py-4 bg-indigo-600 text-white font-semibold rounded-xl hover:bg-indigo-700 focus:ring-4 focus:ring-indigo-300 transition"
                >
                    Primary
                </button>
                <button
                    class="px-8 py-4 border-2 border-indigo-600 text-indigo-600 font-semibold rounded-xl hover:bg-indigo-50 transition"
                >
                    Secondary
                </button>
            </div>
        </div>
    </div>
</template>

<script setup>
// empty is fine
</script>
