# מחקר עומק — MetaHuman (טכני) + שיתוף פעולה Claude Code ↔ TouchDesigner

## חלק א' — MetaHuman: איך זה עובד בפועל (דאטה טכנית)

### הליבה: Rig Logic
כל MetaHuman רץ על **Rig Logic** — runtime של Epic (פותח ע"י 3Lateral, הצוות שמאחורי הפנים ב-Senua's Saga: Hellblade II ו-Marvel's Spider-Man). זה solver קליל וניתן להעברה (portable) שממיר כמה מאות **בקרות בעלות שם סמנטי** (control inputs — למשל "הרם גבה שמאל") ל:
1. טרנספורמציות מפרקים (joint transforms)
2. **shape animation** — הזזת per-vertex ב-LOD0 בלבד (הרזולוציה הגבוהה ביותר, לביצועים)
3. משקלי מפת-אנימציה לבלנדינג קמטים (wrinkle maps) דרך החומרים (materials)

**מספרים קונקרטיים**: ~700–800 מפרקי פנים (תלוי בארכיטיפ), rig הפנים עצמו רץ על **72 עצמות** שמונעות ע"י blend shapes בזמן ה-capture. Solver-רץ ב-**30fps ומעלה** — זה מה שהופך את זה לישים ל-real-time (בניגוד לפייפליינים קולנועיים איטיים יותר).

### פייפליין ה-capture (איך מקבלים תנועה על הפנים)
שתי מערכות נפרדות עובדות **במקביל**:
- **Face capture** → מניע את rig הפנים (72 העצמות) דרך blend shapes.
- **Body animation** → מניע את שלד ה-Mannequin (הגוף), בנפרד לגמרי.

### יצירת MetaHuman מתמונה/סריקה
- **מרובה-תמונות (מדויק)**: KeenTools FaceBuilder — סריקת iPhone טיפוסית ~40 תמונות מזוויות שונות, בונה מש מדויק, מכניסים אותו כ-Mesh-to-MetaHuman.
- **תמונה בודדת (AI, פחות מדויק)**: כלים חדשים יותר "מנחשים" עומק/צורה מ-2D אחד.
- החל מ-**UE 5.6**, MetaHuman Creator משולב ישירות במנוע (לא אפליקציית-ווב נפרדת כמו קודם).

### ייצוא/אינטרופרביליות — **הדבר הכי חשוב לנו**
- ייצוא מ-Unreal ל-**FBX + Alembic**: קליק ימני על ה-skeletal mesh (גוף/פנים) ב-Content Browser → Asset Actions → Export → FBX.
- **מגבלה חשובה**: שיער/גבות/ריסים (hair grooms) **לא** יוצאים ב-FBX רגיל — צריך Alembic נפרד עבורם.
- ל-**Blender**: File → Import → FBX, עם "Import Morph Targets" מופעל.
- **ל-TouchDesigner**: יש **FBX COMP** מובנה שקורא קבצי FBX ישירות — תומך במודלים, אנימציות, וטקסטורות/תמונות. FBX הוא פורמט Autodesk תקני שנתמך גם ב-Blender/Houdini/Maya/Unreal — כלומר **אפשר לייצא MetaHuman מ-Unreal ולהכניס אותו ישירות ל-TD דרך FBX COMP**, בלי Unreal בכלל בזמן הריצה (רק Unreal פעם אחת ליצירת/ייצוא המודל). זה פותח אפשרות אמיתית: לבנות פנים ב-MetaHuman **פעם אחת**, לייצא FBX, ולהריץ עליהם את כל אפקטי הגליץ' שלנו (instancing, wireframe, displacement) **ישירות ב-TD** — בלי תלות ב-Unreal בזמן אמת.

---

## חלק ב' — Claude Code ↔ TouchDesigner: מה כבר קיים ומה אפשר

### אימות חשוב: זה לא ניסוי בודד — יש אקוסיסטם קהילתי פעיל
מצאתי כמה מקורות שמאשרים ש**בדיוק מה שאנחנו עושים** (Claude Code + MCP server + TouchDesigner) הוא כבר תבנית עבודה מוכרת בקהילת TD:
- פוסטים בפורום הרשמי של Derivative (יצרני TD) בדיוק על "Claude Code + MCP bridge + TouchDesigner".
- ריפו GitHub ציבורי (`johnsabath/touchdesigner-mcp`) — "General-purpose MCP server for TouchDesigner — gives AI agents full control over any TD project" — כנראה מאותה משפחה כמו השרת שכבר מחובר אצלנו.
- יש גם "**TouchDesigner Guide**" — Claude Code Skill ייעודי שהופך את Claude ל"עוזר תכנות ויזואלי מתמחה", שאוכף שימוש ב-utility מותאם-אישית בשם `op.TDAPI` כדי להבטיח שיצירת אופרטורים/פריסת רשת/הגדרת פרמטרים תהיה מדויקת ותואמת best-practices. **worth לבדוק אם יש לנו גישה ל-skill כזה או לבנות מוסכמה דומה** (למשל: קונבנציות שמות/מיקום קבועות לצמתים חדשים בפרויקט שלנו).

### מה זה בפועל מאפשר (מאושר ע"י המקורות, לא השערה)
- ה-MCP הופך את Claude ל**שותף יצירתי בתוך TD עצמו** — לא רק "כותב קוד שרץ מבחוץ" — יוצר אופרטורים, מחבר אותם, קורא שגיאות, מריץ פייתון, הכל משפה טבעית.
- **שימוש בפרפורמנס חי**: "כשמרגישים שרוצים שהאפקט יהיה יותר אינטנסיבי, פשוט מקלידים לקלוד עם MCP מחובר לפרויקט TD חי" — כלומר יש כבר תקדים לשימוש בזמן אמת/הופעה, לא רק בפיתוח אופליין. **זה בדיוק הכיוון שיכול לשרת אותנו**: לתת לי גישה חיה תוך כדי הופעה כדי לכוונן פרמטרים לפי הרגשה, לא רק בסשנים מתוכננים.

### הקשר רחב יותר: איפה זה עומד ב-2026 (agentic AI trends)
- המעבר הכללי בתעשייה הוא **מ"כתיבת קוד" ל"תזמור סוכנים שכותבים קוד"** — כלומר הערך עובר ליכולת לתאם/לכוון סוכן AI נכון, לא רק לכתוב כל שורה בעצמך. זה בדיוק המודל שאנחנו כבר עובדים בו (את מנחה כיוון אמנותי, אני מבצע ומאמת).
- "AI פועל פחות כתחליף לאמנים ויותר כמאיץ workflow" — מגמה כללית בתעשיית האנימציה/גיים-ארט ל-2026, תואמת את איך שאנחנו כבר עובדים (לא AI "מייצר לבד", אלא מבצע בדיוק לפי כיוון אמנותי + מאמת אמפירית).

---

## מקורות

- [MetaHuman — The Tech Behind MetaHuman Creator Face Rigs (רשמי)](https://metahuman.com/en-US/learning/the-tech-behind-metahuman-creator-face-rigs)
- [UE5 MetaHuman: What It Is, How It Works, and What It Can't Do](https://nastyrodent.com/ue5-metahuman/)
- [MetaHuman 5.6/5.7: Pipeline Reference — Medium](https://medium.com/@Jamesroha/metahuman-5-6-5-7-pipeline-reference-170d302b078e)
- [Export MetaHuman To Blender: Comprehensive Guide](https://yelzkizi.org/export-metahuman-to-blender/)
- [Fully Exporting a Metahuman from UE5.6 to Blender — Medium](https://medium.com/@little_michael101/fully-exporting-a-metahuman-from-u-e-5-6-to-blender-except-hair-assets-2dc48f12c228)
- [FBX COMP — TouchDesigner Documentation](https://docs.derivative.ca/FBX_COMP)
- [WIP: Claude Code + MCP bridge + TouchDesigner — Derivative community](https://derivative.ca/community-post/asset/wip-openclaw-mcp-bridge-touchdesigner/74168)
- [GitHub — johnsabath/touchdesigner-mcp](https://github.com/johnsabath/touchdesigner-mcp)
- [TouchDesigner Guide — Claude Code Skill (mcpmarket.com)](https://mcpmarket.com/tools/skills/touchdesigner-guide)
- [Claude × TouchDesigner MCP — Clauder Navi](https://www.clauder-navi.com/en/claude-touchdesigner)
- [TouchDesigner MCP Server: The Complete Guide 2026 — MCP.Directory](https://mcp.directory/blog/touchdesigner-mcp-complete-guide-2026)
- [Top 13 Agentic AI Trends to Watch in 2026 — Firecrawl](https://www.firecrawl.dev/blog/agentic-ai-trends)
- [5 Emerging Trends in Animation, VFX & Game Art for 2026 — VanArts](https://www.vanarts.com/news-article/5-emerging-trends-in-animation-vfx-s-game-art-for-2026/)
