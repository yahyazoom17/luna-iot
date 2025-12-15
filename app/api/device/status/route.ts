import type { StatusType } from "@/types/device";
import { NextResponse } from "next/server";
import {
  getDeviceStatus,
  createDeviceStatus,
  updateDeviceStatus,
  deleteDeviceStatus,
} from "@/appwrite/database";

export async function GET() {
  try {
    const res = await getDeviceStatus("Luna#12345");
    const data = res.rows[0];
    return new NextResponse(JSON.stringify(data), {
      status: 200,
      headers: { "Content-Type": "application/json" },
    });
  } catch (error) {
    return new NextResponse(`Error fetching device status: ${error}`, {
      status: 500,
      headers: { "Content-Type": "text/plain" },
    });
  }
}

export async function POST(req: Request) {
  try {
    const body = await req.json();
    const { deviceId, batteryLevel, wifiStrength } = body;
    const newStatus: StatusType = {
      deviceId,
      batteryLevel,
      wifiStrength,
      isConnected: true,
    };
    const existingStatusRes = await getDeviceStatus(deviceId);
    if (existingStatusRes.total > 0) {
      const existingStatus = existingStatusRes.rows[0];
      await updateDeviceStatus(existingStatus.$id, newStatus);
      return new NextResponse(JSON.stringify(newStatus), {
        status: 200,
        headers: { "Content-Type": "application/json" },
      });
    } else {
      await createDeviceStatus(newStatus);
      return new NextResponse(JSON.stringify(newStatus), {
        status: 201,
        headers: { "Content-Type": "application/json" },
      });
    }
  } catch (error) {
    return new NextResponse(`Error updating device status: ${error}`, {
      status: 500,
      headers: { "Content-Type": "text/plain" },
    });
  }
}

export async function PUT(req: Request) {
  try {
    const body = await req.json();
    const { deviceId, batteryLevel, wifiStrength } = body;
    const updatedStatus: StatusType = {
      deviceId,
      batteryLevel,
      wifiStrength,
      isConnected: true,
    };
    const existingStatusRes = await getDeviceStatus(deviceId);
    if (existingStatusRes.total > 0) {
      const existingStatus = existingStatusRes.rows[0];
      await updateDeviceStatus(existingStatus.$id, updatedStatus);
      return new NextResponse(JSON.stringify(updatedStatus), {
        status: 200,
        headers: { "Content-Type": "application/json" },
      });
    } else {
      return new NextResponse(`Device ${deviceId} not found`, {
        status: 404,
        headers: { "Content-Type": "text/plain" },
      });
    }
  } catch (error) {
    return new NextResponse(`Error updating device status: ${error}`, {
      status: 500,
      headers: { "Content-Type": "text/plain" },
    });
  }
}

export async function DELETE(req: Request) {
  try {
    const body = await req.json();
    const { deviceId } = body;
    const existingStatusRes = await getDeviceStatus(deviceId);
    if (existingStatusRes.total > 0) {
      const existingStatus = existingStatusRes.rows[0];
      await deleteDeviceStatus(existingStatus.$id);
      return new NextResponse(`Device ${deviceId} disconnected`, {
        status: 200,
        headers: { "Content-Type": "text/plain" },
      });
    } else {
      return new NextResponse(`Device ${deviceId} not found`, {
        status: 404,
        headers: { "Content-Type": "text/plain" },
      });
    }
  } catch (error) {
    return new NextResponse(`Error deleting device status: ${error}`, {
      status: 500,
      headers: { "Content-Type": "text/plain" },
    });
  }
}
