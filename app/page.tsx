import { Button } from "@/components/ui/button";
import { RefreshCcw } from "lucide-react";

const Home = () => {
  return (
    <div className="flex justify-center items-center h-screen">
      <div className="bg-white rounded-2xl p-5 border flex flex-col items-start gap-2 max-w-sm w-full">
        <div className="text-2xl font-semibold px-1 mb-3 flex flex-row justify-between w-full items-center">
          <div>⚡Luna Status</div>
          <div>
            <Button size={"icon-sm"}>
              <RefreshCcw />
            </Button>
          </div>
        </div>
        <div className="flex flex-col items-start w-full">
          <div className="text-lg px-2 my-1 font-medium">
            Device - {"LUNA#001"}
          </div>
          <div className="text-lg px-2 my-1 font-medium">Battery - {"80%"}</div>
          <div className="text-lg px-2 my-1 font-medium">
            Wifi Strength - {"99ping"}
          </div>
          <div className="px-2 mt-5 w-full">
            <Button className="w-full">Disconnect</Button>
          </div>
        </div>
      </div>
    </div>
  );
};

export default Home;
