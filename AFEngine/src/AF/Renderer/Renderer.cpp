#include "afpch.h"
#include "AF/Renderer/Renderer.h"

#include "AF/Renderer/Renderer2D.h"
#include "AF/Renderer/SceneRenderer.h"
#include "AF/Renderer/RenderCommand.h"

#include <glad/glad.h>

namespace AF {

	/**
	 * @struct RendererData
	 * @brief 鐎涙ê鍋嶅〒鍙夌厠閸ｃ劌鍙忕仦鈧悩鑸碘偓浣烘畱閸愬懘鍎寸€圭懓娅?
	 */
	struct RendererData
	{
		// --- 瑜版挸澧犲ú鏄忕┈閸欍儲鐒?(Current State) ---
		Ref<RenderPass> m_ActiveRenderPass;           ///< 瑜版挸澧犲锝呮躬閹笛嗩攽閻ㄥ嫭瑕嗛弻鎾烩偓姘朵壕
		Ref<VertexArray> m_FullscreenQuadVertexArray; ///< 妫板嫮鐤嗛惃鍕弿鐏炲繒绮崚璺哄殤娴ｆ洘鏆熼幑?(NDC Quad)

		// --- 缁崵绮烘妯款吇鐠у嫭绨?(Fallback Resources) ---
		Ref<Material> DefaultMaterial;                ///< 閸忋劌鐪崗婊冪俺閺夋劘宸濋敍宀勬Щ濮濄垺婀崚鍡涘帳閺夋劘宸濋惃鍕⒖娴ｆ挸顕遍懛鏉戠┛濠?
		Ref<Shader> DefaultShader;                    ///< 閸忋劌鐪崗婊冪俺閻偓閼规彃娅?

		// --- 閻樿埖鈧浇鎷烽煪顏冪瑢閹嗗厴娴兼ê瀵?(State Tracking) ---
		int pipelineUnit = 0;                         ///< 瑜版挸澧?Pass 缁狙冨焼閻ㄥ嫮姹楅悶鍡樞担宥呬焊缁夊鍣?
		int materialUnit = 0;                         ///< 瑜版挸澧犻弶鎰窛缁狙冨焼閻ㄥ嫮姹楅悶鍡樞担宥呬焊缁夊鍣?
		TextureUnitCache textureUnitCache;            ///< 缁惧湱鎮婂Σ鎴掔秴缂傛挸鐡ㄩ敍宀€鏁ゆ禍搴℃躬閸氬奔绔?Draw Call 閸愬懎骞撻柌宥囨睏閻炲棛绮︾€?

		Ref<Mesh> m_ActiveMesh = nullptr;             ///< 缂傛挸鐡ㄦ稉濠佺濞嗏剝瑕嗛弻鎾舵畱缂冩垶鐗搁敍宀€鏁ゆ禍搴″櫤鐏忔垵鍟戞担娆戞畱 VAO 閸掑洦宕叉稉?Uniform 鎼存梻鏁?
		Ref<Material> m_ActiveMaterial = nullptr;     ///< 缂傛挸鐡ㄦ稉濠佺濞嗏剝瑕嗛弻鎾舵畱閺夋劘宸濋敍宀€鏁ゆ禍搴″櫤鐏忔垵鍟戞担娆戞畱 Shader 閸欏倹鏆熼弴瀛樻煀
	};

	static RendererData s_Data; // 閸愬懘鍎撮崗銊ョ湰閸楁洑绶ラ弫鐗堝祦

	// ===================================================================================
	// --- 閸╄櫣顢呴悽鐔锋嚒閸涖劍婀＄粻锛勬倞 ---
	// ===================================================================================

	/**
	 * @brief 閸掓繂顫愰崠鏍ㄨ閺屾挾閮寸紒?
	 * 1. 閸氼垰濮?RenderCommand, Renderer2D, SceneRenderer 缁涘鐗宠箛鍐┠侀崸妞尖偓?
	 * 2. 閺嬪嫬缂撳鏇熸惛姒涙顓婚弶鎰窛閿涘矂顣╃拋?Albedo, Metallic, Roughness 缁?PBR 閸欏倹鏆熼妴?
	 * 3. 妫板嫭鐎?NDC 缁屾椽妫块惃鍕弿鐏炲繐娲撴潏鐟拌埌妞ゅ墎鍋ｉ弫鐗堝祦閿涘奔绶靛鎯扮箿濞撳弶鐓嬮崪灞芥倵婢跺嫮鎮婃担璺ㄦ暏閵?
	 */
	void Renderer::Init()
	{
		AF_PROFILE_FUNCTION();

		// 1. 閸掓繂顫愰崠鏍ㄧ壋韫囧啯瑕嗛弻鎾茨侀崸?
		RenderCommand::Init();
		Renderer2D::Init();
		SceneRenderer::Init();

		// 2. 闁板秶鐤嗘妯款吇 PBR 閺夋劘宸濋崣鍌涙殶 (閸忔粌绨抽弬瑙勵攳)
		auto DefaultMaterial = CreateRef<Material>();
		DefaultMaterial->SetUniform("u_Material.AlbedoColor", glm::vec4(0.8f, 0.8f, 0.8f, 1.0f));
		DefaultMaterial->SetUniform("u_Material.Metallic", 0.04f);
		DefaultMaterial->SetUniform("u_Material.Roughness", 0.8f);
		DefaultMaterial->SetUniform("u_Material.AmbientOcclusion", 1.0f);
		DefaultMaterial->SetUniform("u_Material.UseAlbedoMap", 1);
		DefaultMaterial->SetUniform("u_Material.UseNormalMap", 1);
		DefaultMaterial->SetUniform("u_Material.UseMetallicMap", 0);
		DefaultMaterial->SetUniform("u_Material.UseRoughnessMap", 0);
		DefaultMaterial->SetUniform("u_Material.UseAOMap", 0);

		// 鐏忔繆鐦崝鐘烘祰瀵洘鎼搁崘鍛枂閻ㄥ嫬鐔€绾偓鐠愭潙娴橀崠?
		auto albedoTexture = Texture2D::Create("assets/textures/red_brick_diff_4k.jpg");
		auto normalTexture = Texture2D::Create("assets/textures/red_brick_nor_gl_4k.jpg");
		auto armTexture = Texture2D::Create("assets/textures/red_brick_arm_4k.jpg");

		if (albedoTexture && normalTexture && armTexture) {
			DefaultMaterial->SetUniform("u_AlbedoMap", albedoTexture);
			DefaultMaterial->SetUniform("u_NormalMap", normalTexture);
			DefaultMaterial->SetUniform("u_ARMMap", armTexture); // ARM 閺勭姴鐨? R=AO, G=Roughness, B=Metallic
		}
		else {
			AF_CORE_WARN("Renderer: 閺冪姵纭堕崝鐘烘祰姒涙顓荤痪鍦倞閸栧拑绱濋悧鈺€缍嬬亸鍡樻▔缁€杞拌礋姒涙顓婚悘鎷屽閺夋劘宸濋妴?);
		}
		s_Data.DefaultMaterial = DefaultMaterial;

		// 3. 閸掓繂顫愭妯款吇閻偓閼规彃娅?
		s_Data.DefaultShader = Shader::Create("assets/shaders/phong.glsl");

		// 4. 閺嬪嫬缂撻崗銊ョ潌缂佹ê鍩楅崙鐘辩秿娴?(NDC Quad: -1 to 1)
		s_Data.m_FullscreenQuadVertexArray = VertexArray::Create();

		float vertices[] = {
			// 娴ｅ秶鐤?(x, y, z)     // UV (u, v)
			-1.0f, -1.0f, 0.0f,   0.0f, 0.0f,
			 1.0f, -1.0f, 0.0f,   1.0f, 0.0f,
			-1.0f,  1.0f, 0.0f,   0.0f, 1.0f,

			 1.0f, -1.0f, 0.0f,   1.0f, 0.0f,
			 1.0f,  1.0f, 0.0f,   1.0f, 1.0f,
			-1.0f,  1.0f, 0.0f,   0.0f, 1.0f
		};

		Ref<VertexBuffer> vertexBuffer = VertexBuffer::Create(vertices, sizeof(vertices));
		BufferLayout layout = {
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float2, "a_TexCoord" }
		};
		vertexBuffer->SetLayout(layout);
		s_Data.m_FullscreenQuadVertexArray->AddVertexBuffer(vertexBuffer);
	}

	void Renderer::Shutdown()
	{
		// 濞撳懐鎮婇柅鏄忕帆瀵板懏澧跨仦?
	}

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		RenderCommand::SetViewport(0, 0, width, height);
	}

	// ===================================================================================
	// --- 濞撳弶鐓嬮柅姘朵壕 (Render Pass) 濞翠胶鈻奸幒褍鍩?---
	// ===================================================================================

	/**
	 * @brief 瀵偓閸氼垱瑕嗛弻鎾绘▉濞?
	 * 閼卞矁鐭楅敍?
	 * 1. 閸掑洦宕?GPU 濞撳弶鐓嬮惄顔界垼 (Framebuffer)閵?
	 * 2. 缂佹垵鐣捐ぐ鎾冲 Pass 鐎电懓绨查惃?Shader閵?
	 * 3. 闁插秶鐤嗙痪鍦倞濡叉垝缍呴崪宀€濮搁幀浣芥嫹闊亞绱︾€涙﹫绱濈涵顔荤箽瑜版挸澧?Pass 閻ㄥ嫮濮搁幀浣哄缁斿鈧?
	 * 4. 鎼存梻鏁ょ仦鐐扮艾閺佺繝閲?Pass 閻ㄥ嫬鍙忕仦鈧崣鍌涙殶閿涘牆顩ч幎鏇炲閻晠妯€閵嗕胶骞嗘晶鍐ㄥ帨閿涘鈧?
	 */
	void Renderer::BeginRenderPass(const Ref<RenderPass> renderPass)
	{
		s_Data.m_ActiveRenderPass = renderPass;

		// 1. 濠碘偓濞茬粯瑕嗛弻鎾舵窗閺?
		renderPass->GetSpecification().TargetFramebuffer->Bind();

		// 2. 绾喖鐣鹃獮鑸电负濞?Shader
		Ref<Shader> shader = s_Data.m_ActiveRenderPass->GetSpecification().m_Shader;
		if (!shader) shader = s_Data.DefaultShader;
		shader->Bind();

		// 3. 閻樿埖鈧線鍣哥純顕嗙窗娑撳搫缍嬮崜?Pass 閻ㄥ嫮顑囨稉鈧▎锛勭帛閸掕泛浠涢崙鍡楊槵
		s_Data.pipelineUnit = 0;
		s_Data.materialUnit = 0;
		s_Data.textureUnitCache.clear();
		s_Data.m_ActiveMesh = nullptr;
		s_Data.m_ActiveMaterial = nullptr;

		// 4. 鎼存梻鏁ら崗銊ョ湰 Pass Uniform (isGlobal=true)
		renderPass->Apply(shader, true, s_Data.pipelineUnit, s_Data.materialUnit, s_Data.textureUnitCache);
	}

	void Renderer::EndRenderPass()
	{
		// 鐟欙絿绮﹁ぐ鎾冲濞撳弶鐓嬮惄顔界垼
		s_Data.m_ActiveRenderPass->GetSpecification().TargetFramebuffer->Unbind();

		// 闁插秶鐤嗙痪鍦倞鐠佲剝鏆熼敍宀勬Щ濮濄垼娉曢柅姘朵壕缁惧湱鎮婂Ч鈩冪厠
		s_Data.pipelineUnit = 0;
		s_Data.materialUnit = 0;
	}

	// ===================================================================================
	// --- 閸戠姳缍嶆担鎾村絹娴?(Drawing Submission) ---
	// ===================================================================================

	/**
	 * @brief 閹笛嗩攽缂冩垶鐗稿〒鍙夌厠
	 * 闁俺绻冮垾婊呭Ц閹浇鎷烽煪顏佲偓婵囧Η閺堫垯绱崠鏍ㄢ偓褑鍏橀敍?
	 * - 婵″倹鐏夋潻鐐电敾缂佹ê鍩楁担璺ㄦ暏閻╃鎮撻弶鎰窛閻ㄥ嫮澧挎担鎿勭礉鐠哄疇绻冮弶鎰窛缁?Uniform 閻ㄥ嫰鍣搁弬鎵拨鐎规哎鈧?
	 * - 婵″倹鐏夋潻鐐电敾缂佹ê鍩楅惄绋挎倱缂冩垶鐗搁敍宀冪儲鏉╁洨缍夐弽鑲╅獓 Uniform 閻ㄥ嫰鍣搁弬鎵拨鐎规哎鈧?
	 */
	void Renderer::SubmitMesh(const Ref<Mesh>& mesh, const Ref<Material>& overridematerial, const UniformContainer& instanceUniforms)
	{
		auto& material = overridematerial ? overridematerial : s_Data.DefaultMaterial;
		
		Ref<Shader> shader = s_Data.m_ActiveRenderPass->GetSpecification().m_Shader;
		if (!shader) shader = s_Data.DefaultShader;

		// 1. 鎼存梻鏁ら弶鎰窛閸欏倹鏆?(娴犲懎婀弶鎰窛鐎圭偘绶ラ崚鍥ㄥ床閺冭埖澧界悰?
		if (s_Data.m_ActiveMaterial != material) {
			s_Data.materialUnit = 0; // 濮ｅ繋閲滈弶鎰窛娴?0 閸欓攱娼楃拹銊ф睏閻炲棗宕熼崗鍐ㄧ磻婵顓搁弫?
			material->Apply(shader, false, s_Data.pipelineUnit, s_Data.materialUnit, s_Data.textureUnitCache);
			s_Data.m_ActiveMaterial = material;
		}

		// 2. 鎼存梻鏁ょ純鎴炵壐閸欏倹鏆?(娴犲懎婀純鎴炵壐鐠у嫭绨崚鍥ㄥ床閺冭埖澧界悰?
		if (s_Data.m_ActiveMesh != mesh) {
			mesh->Apply(shader, false, s_Data.pipelineUnit, s_Data.materialUnit, s_Data.textureUnitCache);
			s_Data.m_ActiveMesh = mesh;
		}

		// 3. 鎼存梻鏁ょ€圭偘绶ョ粔浣规箒閸欏倹鏆?(婵″倸褰夐幑銏㈢叐闂冪绱濊箛鍛淬€忓В蹇旑偧閺囧瓨鏌?
		instanceUniforms.Apply(shader, false, s_Data.pipelineUnit, s_Data.materialUnit, s_Data.textureUnitCache);

		// 4. 濞叉儳褰?Draw Call
		Renderer::Submit([=]()
			{
				RenderCommand::DrawIndexed(mesh->m_VertexArray, mesh->m_IndexCount);
			});
	}

	/**
	 * @brief 閹绘劒姘﹂梼鏉戝缂佹ê鍩?(缁墽鐣濋悧?SubmitMesh)
	 * 娴犲懎顦╅悶鍡欑秹閺嶇厧鍤戞担鏇氫繆閹垰鎷版担宥囩枂閸欐ɑ宕查敍灞芥嫹閻ｃ儱鍘滈悡褋鈧線顤侀懝鑼搼閺夋劘宸濋崣鍌涙殶閿涘本鐎径褍鍣虹亸鎴︽Ь瑜拌鲸瑕嗛弻鎾绘▉濞堢數娈?CPU/GPU 瀵偓闁库偓閵?
	 */
	void Renderer::SubmitShadow(const Ref<Mesh>& mesh, const UniformContainer& instanceUniforms)
	{
		Ref<Shader> shader = s_Data.m_ActiveRenderPass->GetSpecification().m_Shader;
		if (!shader) shader = s_Data.DefaultShader;

		s_Data.materialUnit = 0;
		
		// 1. 缂冩垶鐗搁崚鍥ㄥ床濡偓閺?
		if (s_Data.m_ActiveMesh != mesh) {
			mesh->Apply(shader, false, s_Data.pipelineUnit, s_Data.materialUnit, s_Data.textureUnitCache);
			s_Data.m_ActiveMesh = mesh;
		}

		// 2. 鎼存梻鏁ら崣妯诲床閻晠妯€
		instanceUniforms.Apply(shader, false, s_Data.pipelineUnit, s_Data.materialUnit, s_Data.textureUnitCache);

		// 3. 缁便垹绱╃紒妯哄煑
		Renderer::Submit([=]()
			{
				RenderCommand::DrawIndexed(mesh->m_VertexArray, mesh->m_IndexCount);
			});
	}

	void Renderer::SubmitFullscreenQuad()
	{
		// 缂佹ê鍩楁０鍕€铏规畱閸忋劌鐫嗛崶娑滅珶瑜邦澁绱濋柅姘埗閸︺劌娆㈡潻鐔歌閺屾挸鍘滈悡褔妯佸▓浣冪殶閻?
		RenderCommand::DrawTriangles(s_Data.m_FullscreenQuadVertexArray, 6);
	}

	/**
	 * @brief 閹稿洣鎶ら幍褑顢戞稉顓熺亼
	 * 鐠愮喕鐭楃亸鍡樿閺屾捁顕Ч鍌涙烦閸欐垵鍩岄崗铚傜秼閻ㄥ嫭瑕嗛弻鎾虫嚒娴犮倧绱欒ぐ鎾冲娑撹櫣娲块幒銉﹀⒔鐞涘矉绱濋張顏呮降閸欘垱鏁幐浣告嚒娴犮倖甯撴惔蹇庣瑢婢舵氨鍤庣粙瀣絹娴溿倧绱氶妴?
	 */
	void Renderer::Submit(const std::function<void()>& renderFunc)
	{
		renderFunc();
	}

}
