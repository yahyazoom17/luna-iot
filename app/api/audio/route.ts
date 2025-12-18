import { writeFile } from "fs/promises";
import path from "path";

export async function POST(req: Request) {
  const buffer = Buffer.from(await req.arrayBuffer());

  const filePath = path.join(
    process.cwd(),
    "public",
    `audio-${Date.now()}.wav`
  );

  await writeFile(filePath, buffer);

  return Response.json({ success: true });
}
