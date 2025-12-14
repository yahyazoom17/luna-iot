"use client";
import { Button } from "@/components/ui/button";
import { RefreshCcw, Zap, Wifi, Battery, Cpu, ZapOff } from "lucide-react";
import { Badge } from "@/components/ui/badge";
import { useState } from "react";

const StatusCard = () => {
  const [connectionStatus, setConnectionStatus] = useState(false);

  const toggleConnection = () => {
    setConnectionStatus(!connectionStatus);
  };

  return (
    <div className="bg-white rounded-2xl p-5 border flex flex-col items-start gap-2 max-w-sm w-full">
      <div className="text-2xl font-semibold px-1 mb-3 flex flex-row justify-between w-full items-center">
        <div className="flex items-center justify-between">
          <div>🤖 Luna Status</div>
          <div>
            {connectionStatus ? (
              <Badge className="ml-2 flex items-center gap-1 bg-green-100 text-green-800">
                <Zap size={50} />
                Connected
              </Badge>
            ) : (
              <Badge className="ml-2 flex items-center gap-1 bg-red-100 text-red-800">
                <ZapOff size={50} />
                Disconnected
              </Badge>
            )}
          </div>
        </div>
        <div>
          <Button size={"icon-sm"}>
            <RefreshCcw />
          </Button>
        </div>
      </div>
      <div className="flex flex-col items-start w-full">
        <div className="text-md px-2 my-1 font-medium flex items-center flex-row gap-2">
          <Cpu size={"15"} />
          <div>Device - {"LUNA #001"}</div>
        </div>
        <div className="text-md px-2 my-1 font-medium flex items-center flex-row gap-2">
          <Battery size={"15"} />
          <div>Battery - {"80%"}</div>
        </div>
        <div className="text-md px-2 my-1 font-medium flex items-center flex-row gap-2">
          <Wifi size={"15"} />
          <div>Wifi Strength - {"99ping"}</div>
        </div>
        <div className="px-2 mt-5 w-full">
          <Button size={"sm"} className="w-full" onClick={toggleConnection}>
            {connectionStatus ? <ZapOff size={"15"} /> : <Zap size={"15"} />}
            {connectionStatus ? "Disconnect" : "Connect"}
          </Button>
        </div>
      </div>
    </div>
  );
};

export default StatusCard;
