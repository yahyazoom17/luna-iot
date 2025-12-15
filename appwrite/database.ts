import { StatusType } from "@/types/device";
import { databases } from "./appwrite";
import { ID, Query } from "appwrite";

const databaseId = process.env.NEXT_PUBLIC_APPWRITE_DATABASE_ID as string;
const tableId = "status";

export const getDeviceStatus = async (deviceId: string) => {
  return await databases.listRows({
    databaseId,
    tableId,
    queries: [Query.equal("deviceId", deviceId)],
  });
};

export const createDeviceStatus = async (status: StatusType) => {
  return await databases.createRow({
    databaseId,
    tableId,
    rowId: ID.unique(),
    data: status,
  });
};

export const updateDeviceStatus = async (
  deviceId: string,
  newStatus: StatusType
) => {
  return await databases.updateRow({
    databaseId,
    tableId,
    rowId: deviceId,
    data: newStatus,
  });
};

export const deleteDeviceStatus = async (deviceId: string) => {
  return await databases.deleteRow({
    databaseId,
    tableId,
    rowId: deviceId,
  });
};
