import { characterById } from '@/lib/characters';
export default function CharacterIcon({
  id = 'mochi',
  className = '',
}: {
  id?: string;
  className?: string;
}) {
  const c = characterById(id);
  return (
    <svg viewBox="-195 -165 390 310" className={className} aria-hidden="true">
      <g fill={c.body}>
        {id === 'peach' && (
          <>
            <path d="M-115-44 Q-132-104-109-132 Q-75-134-48-83Z" />
            <path d="M115-44 Q132-104 109-132 Q75-134 48-83Z" />
          </>
        )}
        {id === 'nimbus' ? (
          <>
            <ellipse cy="30" rx="147" ry="94" />
            <ellipse cx="-94" cy="-13" rx="58" ry="70" />
            <ellipse cx="-27" cy="-58" rx="67" ry="66" />
            <ellipse cx="68" cy="-30" rx="68" ry="73" />
            <ellipse cx="117" cy="24" rx="38" ry="55" />
          </>
        ) : (
          <path
            d={`M0 ${id === 'mochi' ? -128 : -100} C79 -130 123 -69 145 -3 C161 42 171 80 146 101 C124 122 59 124 0 124 C-62 124 -127 121 -148 103 C-174 83 -162 42 -145 -5 C-122 -75 -72 -129 0 ${id === 'mochi' ? -128 : -100}Z`}
          />
        )}
        <ellipse cx="-151" cy="-2" rx="21" ry="29" transform="rotate(-22 -151 -2)" />
        <ellipse cx="153" cy="45" rx="20" ry="28" />
      </g>
      {id === 'sprout' && (
        <g fill={c.accent}>
          <path d="M0-96 Q-8-115 4-129 L8-127 Q0-113 6-96Z" />
          <path d="M2-113 C-5-139-28-140-46-127 C-34-111-17-103 2-113Z" />
          <path d="M2-113 C5-139 28-140 46-138 C34-111 17-103 2-113Z" />
        </g>
      )}
      {id === 'peach' && (
        <g fill={c.accent}>
          <path d="M-105-83 L-105-118 L-73-88Z" />
          <path d="M105-83 L105-118 L73-88Z" />
        </g>
      )}
      <g fill={c.face}>
        <rect x="-56" y="-31" width="17" height="34" rx="8.5" />
        <rect x="40" y="-31" width="17" height="34" rx="8.5" />
      </g>
      <g fill={c.blush}>
        <ellipse cx="-68" cy="17" rx="13" ry="8.5" />
        <ellipse cx="68" cy="17" rx="13" ry="8.5" />
      </g>
      <path
        d="M-12 19 C-6 31 6 31 12 18"
        stroke={c.face}
        strokeWidth="5"
        strokeLinecap="round"
        fill="none"
      />
      {id === 'peach' && (
        <g stroke={c.crease} strokeWidth="2.2">
          <path d="M-93 5L-106 2M-93 15L-106 12M93 5L106 2M93 15L106 12" />
        </g>
      )}
    </svg>
  );
}
