// src/services/api.js
import axios from "axios";

const api = axios.create({
    baseURL: "http://127.0.0.1:8000",
    headers: {
        "Content-Type": "application/json",
    },
    // responseType: "blob" → 이제 필요 없음 (기본 json)
});

export const crawlWebsites = async (payload) => {
    try {
        const response = await api.post("/api/crawl/", payload);

        // JSON 응답 처리
        return {
            success: true,
            data: response.data,
        };
    } catch (error) {
        console.error("Crawl error:", error);

        let errMsg = "Network error";
        if (error.response?.data) {
            if (typeof error.response.data === "string") {
                errMsg = error.response.data;
            } else if (error.response.data?.error) {
                errMsg = error.response.data.error;
            } else {
                errMsg = JSON.stringify(error.response.data);
            }
        }

        return {
            success: false,
            error: errMsg,
        };
    }
};
