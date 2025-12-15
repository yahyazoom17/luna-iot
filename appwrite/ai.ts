import type { LlmChatType } from "@/types/llm";
import { databases } from "./appwrite";
import { ID, Query } from "appwrite";

const databaseId = process.env.NEXT_PUBLIC_APPWRITE_DATABASE_ID as string;
const tableId = "chats";

export const getChats = async (deviceId: string) => {
  return await databases.listRows({
    databaseId,
    tableId,
    queries: [Query.equal("deviceId", deviceId)],
  });
};

export const createChat = async (chat: LlmChatType) => {
  return await databases.createRow({
    databaseId,
    tableId,
    rowId: ID.unique(),
    data: chat,
  });
};

export const deleteChat = async (chatId: string) => {
  return await databases.deleteRow({
    databaseId,
    tableId,
    rowId: chatId,
  });
};
