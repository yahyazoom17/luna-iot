"use client";
import { Button } from "@/components/ui/button";
import {
  RefreshCcw,
  Zap,
  Wifi,
  WifiOff,
  Battery,
  Cpu,
  ZapOff,
} from "lucide-react";
import { Badge } from "@/components/ui/badge";
import { useState } from "react";
import { StatusType } from "@/types/device";
import { useEffect } from "react";

const StatusCard = () => {
  const [deviceStatus, setDeviceStatus] = useState<StatusType>();
  const [isLoading, setIsLoading] = useState<boolean>(false);

  const initialStatus: StatusType = {
    deviceId: "No device connected",
    batteryLevel: 0,
    wifiStrength: 0,
    isConnected: false,
  };

  useEffect(() => {
    const connectToWebSocket = () => {
      setIsLoading(true);
      const ws = new WebSocket("ws://localhost:3001");
      console.log("Connected to WebSocket!");
      ws.onmessage = (event) => {
        setDeviceStatus(JSON.parse(event.data));
      };
      setIsLoading(false);
      return () => ws.close();
    };
    connectToWebSocket();
  }, []);

  const fetchLunaStatus = async () => {
    try {
      const response = await fetch("/api/device/status");
      const data = await response.json();
      setDeviceStatus(data);
      console.log("Device Status:", data);
    } catch (error) {
      console.error("Error fetching device status:", error);
    }
  };

  const toggleConnection = () => {
    if (deviceStatus?.isConnected) {
      setDeviceStatus(initialStatus);
    } else {
      fetchLunaStatus();
    }
  };

  const wifiStatus = () => {
    if (deviceStatus?.wifiStrength) {
      if (deviceStatus.wifiStrength > -50) {
        return "Excellent";
      } else if (deviceStatus.wifiStrength > -70) {
        return "Good";
      } else if (deviceStatus.wifiStrength > -80) {
        return "Poor";
      } else {
        return "Very Poor";
      }
    }
  };

  return (
    <div className="bg-white rounded-2xl p-5 border flex flex-col items-start gap-2 max-w-sm w-full">
      <div className="text-2xl font-semibold px-1 mb-3 flex flex-row justify-between w-full items-center">
        <div className="flex items-center justify-between">
          <div>🤖 Luna Status</div>
          <div>
            {deviceStatus?.isConnected ? (
              <Badge className="ml-2 flex items-center gap-1 bg-green-100 text-green-800">
                <Wifi size={50} />
                Connected
              </Badge>
            ) : (
              <Badge className="ml-2 flex items-center gap-1 bg-red-100 text-red-800">
                <WifiOff size={50} />
                {isLoading ? "Connecting to Luna..." : "Disconnected"}
              </Badge>
            )}
          </div>
        </div>
        <div>
          <Button size={"icon-sm"} onClick={fetchLunaStatus}>
            <RefreshCcw />
          </Button>
        </div>
      </div>
      <div className="flex flex-col items-start w-full">
        <div className="text-md px-2 my-1 font-medium flex items-center flex-row gap-2">
          <Cpu size={"15"} />
          <div>Device = {deviceStatus?.deviceId || "No device connected"}</div>
        </div>
        <div className="text-md px-2 my-1 font-medium flex items-center flex-row gap-2">
          <Battery size={"15"} />
          <div>Battery = {deviceStatus?.batteryLevel || 0}%</div>
        </div>
        <div className="text-md px-2 my-1 font-medium flex items-center flex-row gap-2">
          <Wifi size={"15"} />
          <div>
            Wifi Strength = {deviceStatus?.wifiStrength || 0} dBm{" "}
            <span
              className={`${
                deviceStatus?.wifiStrength && deviceStatus.wifiStrength > -50
                  ? "text-green-600"
                  : deviceStatus?.wifiStrength &&
                    deviceStatus.wifiStrength > -70
                  ? "text-yellow-600"
                  : "text-red-600"
              } font-semibold text-xs`}
            >
              ({deviceStatus?.wifiStrength ? wifiStatus() : "No Signal"})
            </span>
          </div>
        </div>
        <div className="px-2 mt-5 w-full">
          <Button size={"sm"} className="w-full" onClick={toggleConnection}>
            {deviceStatus?.isConnected ? (
              <ZapOff size={"15"} />
            ) : (
              <Zap size={"15"} />
            )}
            {isLoading && "Connecting to LUNA..."}
            {deviceStatus?.isConnected ? "Disconnect" : "Connect"}
          </Button>
        </div>
      </div>
    </div>
  );
};

export default StatusCard;
