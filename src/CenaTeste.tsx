import { ThreeCanvas } from "@remotion/three";
import { AbsoluteFill, useCurrentFrame, useVideoConfig } from "remotion";

// Cubo 3D girando. A rotação depende só do número do quadro (frame),
// assim o vídeo final sai igual à prévia, sem tremidas.
export const CenaTeste: React.FC = () => {
  const frame = useCurrentFrame();
  const { width, height, durationInFrames } = useVideoConfig();

  // Uma volta completa (2π) ao longo do vídeo inteiro: o loop fica perfeito.
  const volta = (frame / durationInFrames) * Math.PI * 2;

  return (
    <AbsoluteFill style={{ backgroundColor: "#0f1020" }}>
      <ThreeCanvas
        width={width}
        height={height}
        camera={{ position: [0, 0, 6], fov: 50 }}
      >
        <ambientLight intensity={0.4} />
        <directionalLight position={[5, 5, 5]} intensity={1.2} />
        <mesh rotation={[0.5 + volta * 0.5, 0.7 + volta, 0]}>
          <boxGeometry args={[2, 2, 2]} />
          <meshStandardMaterial color="#4a9eff" />
        </mesh>
      </ThreeCanvas>
    </AbsoluteFill>
  );
};
