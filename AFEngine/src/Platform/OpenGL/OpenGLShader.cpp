#include "afpch.h"
#include "Platform/OpenGL/OpenGLShader.h"
#include "AF/Core/Timer.h"

#include <fstream>

#include <glm/gtc/type_ptr.hpp>

namespace AF {

	namespace Utils {

		static GLenum ShaderTypeFromString(const std::string& type)
		{
			if (type == "vertex")
				return GL_VERTEX_SHADER;
			if (type == "fragment" || type == "pixel")
				return GL_FRAGMENT_SHADER;

			AF_CORE_ASSERT(false, "Unknown shader type!");
			return 0;
		}

		static const char* GetCacheDirectory()
		{
			// TODO: make sure the assets directory is valid
			return "assets/cache/shader/opengl";
		}

		static void CreateCacheDirectoryIfNeeded()
		{
			std::string cacheDirectory = GetCacheDirectory();
			if (!std::filesystem::exists(cacheDirectory))
				std::filesystem::create_directories(cacheDirectory);
		}

	}

	OpenGLShader::OpenGLShader(const std::string& filepath)
	{
		AF_PROFILE_FUNCTION();

		Utils::CreateCacheDirectoryIfNeeded();

		std::string source = ReadFile(filepath);
		auto shaderSources = PreProcess(source);

		Compile(shaderSources);

		//���ļ�·����ȡ��ɫ������
		auto lastSlash = filepath.find_last_of("/\\");
		lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
		auto lastDot = filepath.rfind('.');
		auto count = lastDot == std::string::npos ? filepath.size() - lastSlash : lastDot - lastSlash;
		m_Name = filepath.substr(lastSlash, count);
	}

	OpenGLShader::OpenGLShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc)
		: m_Name(name)
	{
		AF_PROFILE_FUNCTION();

		std::unordered_map<GLenum, std::string> shaderSources;
		shaderSources[GL_VERTEX_SHADER] = vertexSrc;
		shaderSources[GL_FRAGMENT_SHADER] = fragmentSrc;
		Compile(shaderSources);
	}

	OpenGLShader::~OpenGLShader()
	{
		AF_PROFILE_FUNCTION();

		glDeleteProgram(m_RendererID);
	}

	std::string OpenGLShader::ReadFile(const std::string& filepath)
	{
		AF_PROFILE_FUNCTION();

		std::string result;
		std::ifstream in(filepath, std::ios::in | std::ios::binary);
		if (in)
		{
			in.seekg(0, std::ios::end);
			result.resize(in.tellg());
			in.seekg(0, std::ios::beg);
			in.read(&result[0], result.size());
			in.close();
		}
		else
		{
			AF_CORE_ERROR("Could not open file '{0}'", filepath);
		}

		return result;
	}

	std::unordered_map<GLenum, std::string> OpenGLShader::PreProcess(const std::string& source)
	{
		AF_PROFILE_FUNCTION();

		std::unordered_map<GLenum, std::string> shaderSources;

		const char* typeToken = "#type";
		size_t typeTokenLength = strlen(typeToken);
		size_t pos = source.find(typeToken, 0); // ��ɫ�����������еĿ�ʼλ��
		while (pos != std::string::npos)
		{
			size_t eol = source.find_first_of("\r\n", pos); // ��ɫ�����������еĽ���λ��
			AF_CORE_ASSERT(eol != std::string::npos, "Syntax error");
			size_t begin = pos + typeTokenLength + 1; // ��ɫ���������ƵĿ�ʼλ�ã���"#type "�ؼ���֮��
			std::string type = source.substr(begin, eol - begin);
			AF_CORE_ASSERT(Utils::ShaderTypeFromString(type), "Invalid shader type specified");

			size_t nextLinePos = source.find_first_not_of("\r\n", eol); // ��ɫ������������֮�����ɫ�����뿪ʼλ��
			AF_CORE_ASSERT(nextLinePos != std::string::npos, "Syntax error");
			pos = source.find(typeToken, nextLinePos); // ��һ����ɫ�����������еĿ�ʼλ��

			shaderSources[Utils::ShaderTypeFromString(type)] = (pos == std::string::npos)
				                                                   ? source.substr(nextLinePos)
				                                                   : source.substr(nextLinePos, pos - nextLinePos);
		}

		return shaderSources;
	}

	void OpenGLShader::Compile(const std::unordered_map<GLenum, std::string>& shaderSources)
	{
		GLuint program = glCreateProgram();;
		AF_CORE_ASSERT(shaderSources.size() <= 2, "Ŀǰֻ֧��������ɫ��");
		std::array<GLenum, 2> glShaderIDs;
		int glShaderIDIndex = 0;
		for (auto& kv : shaderSources)
		{
			GLenum type = kv.first;
			const std::string source = kv.second;

			// ����һ���յ���ɫ�����
			GLuint Shader = glCreateShader(type);

			// ����ɫ��Դ���봫�ݸ�OpenGL
			// ע�⣺std::string��.c_str()����NULL�ַ���β
			const GLchar* sourceCstr = source.c_str();
			glShaderSource(Shader, 1, &sourceCstr, 0);

			// ������ɫ��
			glCompileShader(Shader);

			// ������״̬
			GLint isCompiled = 0;
			glGetShaderiv(Shader, GL_COMPILE_STATUS, &isCompiled);
			if (isCompiled == GL_FALSE)
			{
				// ��ȡ������־����
				GLint maxLength = 0;
				glGetShaderiv(Shader, GL_INFO_LOG_LENGTH, &maxLength);

				// ������־��������maxLength����NULL��ֹ����
				std::vector<GLchar> infoLog(maxLength);
				glGetShaderInfoLog(Shader, maxLength, &maxLength, &infoLog[0]);

				// ɾ��ʧ�ܵ���ɫ��
				glDeleteShader(Shader);

				// ������Դ��������־�������ӡ�����
				AF_CORE_ERROR("{0}", infoLog.data());
				AF_CORE_ASSERT(false, "Shader compilation failure!");
				break;
			}
			// ������ɫ��������
			glAttachShader(program, Shader);
			glShaderIDs[glShaderIDIndex++] = Shader;
		}

		// ���ӳ���
		glLinkProgram(program);

		// ע������ʹ��glGetProgram*������glGetShader*
		GLint isLinked = 0;
		glGetProgramiv(program, GL_LINK_STATUS, (int*)&isLinked);
		if (isLinked == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

			// ������Դ��ɾ���������ɫ��
			glDeleteProgram(program);

			for (auto id : glShaderIDs)
			{
				glDeleteShader(id);
			}

			// ���������־
			AF_CORE_ERROR("{0}", infoLog.data());
			AF_CORE_ASSERT(false, "Shader link failure!");
			return;
		}

		// ���ӳɹ��󣬷�����ɫ����������Դռ�ã�
		for (auto id : glShaderIDs)
		{
			glDetachShader(program, id);
		}

		m_RendererID = program;

		ExtractUniformReflection();
	}

	void OpenGLShader::ExtractUniformReflection() {
		GLint uniformCount;
		glGetProgramiv(m_RendererID, GL_ACTIVE_UNIFORMS, &uniformCount);

		for (int i = 0; i < uniformCount; i++) {
			char name[256];
			GLsizei length;
			GLint size;
			GLenum type;

			glGetActiveUniform(m_RendererID, i, sizeof(name), &length, &size, &type, name);
			GLint location = glGetUniformLocation(m_RendererID, name);

			m_UniformInfos[name] = { location, type, size };
		}
	}

	void OpenGLShader::Bind() const
	{
		AF_PROFILE_FUNCTION();

		glUseProgram(m_RendererID);
	}

	void OpenGLShader::Unbind() const
	{
		AF_PROFILE_FUNCTION();

		glUseProgram(0);
	}

	void OpenGLShader::SetInt(const std::string& name, int value)
	{
		AF_PROFILE_FUNCTION();

		UploadUniformInt(name, value);
	}

	void OpenGLShader::SetIntArray(const std::string& name, int* values, uint32_t count)
	{
		UploadUniformIntArray(name, values, count);
	}

	void OpenGLShader::SetFloat(const std::string& name, float value)
	{
		AF_PROFILE_FUNCTION();

		UploadUniformFloat(name, value);
	}

	void OpenGLShader::SetFloat2(const std::string& name, const glm::vec2& value)
	{
		AF_PROFILE_FUNCTION();

		UploadUniformFloat2(name, value);
	}

	void OpenGLShader::SetFloat3(const std::string& name, const glm::vec3& value)
	{
		AF_PROFILE_FUNCTION();

		UploadUniformFloat3(name, value);
	}

	void OpenGLShader::SetFloat4(const std::string& name, const glm::vec4& value)
	{
		AF_PROFILE_FUNCTION();

		UploadUniformFloat4(name, value);
	}

	void OpenGLShader::SetMat3(const std::string& name, const glm::mat3& value)
	{
		AF_PROFILE_FUNCTION();

		UploadUniformMat3(name, value);
	}

	void OpenGLShader::SetMat4(const std::string& name, const glm::mat4& value)
	{
		AF_PROFILE_FUNCTION();

		UploadUniformMat4(name, value);
	}

	void OpenGLShader::UploadUniformInt(const std::string& name, int value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1i(location, value);
	}

	void OpenGLShader::UploadUniformIntArray(const std::string& name, int* values, uint32_t count)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1iv(location, count, values);
	}

	void OpenGLShader::UploadUniformFloat(const std::string& name, float value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1f(location, value);
	}

	void OpenGLShader::UploadUniformFloat2(const std::string& name, const glm::vec2& value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform2f(location, value.x, value.y);
	}

	void OpenGLShader::UploadUniformFloat3(const std::string& name, const glm::vec3& value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform3f(location, value.x, value.y, value.z);
	}

	void OpenGLShader::UploadUniformFloat4(const std::string& name, const glm::vec4& value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform4f(location, value.x, value.y, value.z, value.w);
	}

	void OpenGLShader::UploadUniformMat3(const std::string& name, const glm::mat3& matrix)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}

	void OpenGLShader::UploadUniformMat4(const std::string& name, const glm::mat4& matrix)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}

}
