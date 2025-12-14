import type { StatusType } from "@/types/device";
import { NextResponse } from "next/server";
import {
  getDeviceStatus,
  createDeviceStatus,
  updateDeviceStatus,
} from "@/appwrite/database";

export async function GET() {
  const res = await getDeviceStatus("Luna#12345");
  const data = res.rows[0];
  return new NextResponse(JSON.stringify(data), {
    status: 200,
    headers: { "Content-Type": "application/json" },
  });
}

export async function POST(req: Request) {
  const body = await req.json();
  const { deviceId, batteryLevel, wifiStrength } = body;
  const newStatus: StatusType = {
    deviceId,
    batteryLevel,
    wifiStrength,
    isConnected: true,
  };
  return new NextResponse(JSON.stringify(newStatus), {
    status: 201,
    headers: { "Content-Type": "application/json" },
  });
}
