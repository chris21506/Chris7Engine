#pragma once

#include <d3d11.h>
#include <cstdint>
#include <vector>

class MeshComponent final {
public:
    // Estructura de vértice básica para compatibilidad con shaders HLSL
    struct Vertex {
        float position[3];
        float color[4];
        float texCoord[2];
        float normal[3];
    };

    MeshComponent() noexcept;
    ~MeshComponent() noexcept;

    // Prevenir copia
    MeshComponent(const MeshComponent&) = delete;
    MeshComponent& operator=(const MeshComponent&) = delete;

    // Permitir movimiento
    MeshComponent(MeshComponent&&) noexcept;
    MeshComponent& operator=(MeshComponent&&) noexcept;

    /**
     * @brief Inicializa los Vertex e Index buffers del Mesh en la GPU.
     * @param device Dispositivo Direct3D 11 utilizado para crear los búferes.
     * @param vertices Lista de vértices del modelo.
     * @param indices Lista opcional de índices para dibuja mediante DrawIndexed.
     * @return true si la creación de búferes fue exitosa, false en caso contrario.
     */
    bool Initialize(
        ID3D11Device* device,
        const std::vector<Vertex>& vertices,
        const std::vector<std::uint32_t>& indices = {}
    ) noexcept;

    /**
     * @brief Asigna los búferes del Mesh al Pipeline y emite la orden de dibujo (Draw/DrawIndexed).
     * @param context Contexto de renderizado de Direct3D 11.
     */
    void Render(ID3D11DeviceContext* context) noexcept;

    /**
     * @brief Libera los recursos de GPU asociados al Mesh.
     */
    void Shutdown() noexcept;

    // Getters
    [[nodiscard]] ID3D11Buffer* GetVertexBuffer() const noexcept;
    [[nodiscard]] ID3D11Buffer* GetIndexBuffer() const noexcept;
    [[nodiscard]] std::uint32_t GetVertexCount() const noexcept;
    [[nodiscard]] std::uint32_t GetIndexCount() const noexcept;
    [[nodiscard]] bool HasIndices() const noexcept;

private:
    struct Implementation;
    Implementation* m_implementation = nullptr;
};
