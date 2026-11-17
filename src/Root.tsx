import { Composition } from "remotion";
import { CenaTeste } from "./CenaTeste";

export const RemotionRoot: React.FC = () => {
  return (
    <>
      <Composition
        id="CenaTeste"
        component={CenaTeste}
        durationInFrames={150}
        fps={30}
        width={1080}
        height={1920}
      />
    </>
  );
};
